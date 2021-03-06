#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <queue>

#include <arpa/inet.h>

#include "RoutingProtocolImpl.h"
#include "global.h"

//
// Small utilities.
//

Packet parse_packet(Port_id port, void* packet, Packet_size size)
{
    Packet res;

    res.type = static_cast<ePacketType>(*static_cast<uint8_t*>(packet));
    res.port = port;

    auto packet_16 = static_cast<uint16_t*>(packet);
    res.size = ntohs(packet_16[1]);
    assert(res.size == size);
    res.src = ntohs(packet_16[2]);
    res.dest = ntohs(packet_16[3]);

    res.payload = static_cast<void*>(packet_16 + 4);
    res.packet = packet;

    return res;
}

void* prepare_packet(
    ePacketType type, Packet_size size, Router_id src, Router_id dest)
{
    assert(size >= PACKET_HEADER_SIZE);
    auto packet = static_cast<uint8_t*>(malloc(size));
    packet[0] = type;
    packet[1] = 0;

    auto packet_16 = reinterpret_cast<uint16_t*>(packet);
    packet_16[1] = htons(size);
    packet_16[2] = htons(src);
    packet_16[3] = htons(dest);

    return static_cast<void*>(packet);
}

const Packet_size PACKET_HEADER_SIZE = 8;

//
// DV routing method.
//

void DV_router::update_port(
    Port_id port_id, Router_id router_id, Time rtt, Port_event event)
{
    auto updated = compute_dv();
    if (!updated)
        return;
    send_dv();
    return;
}

bool DV_router::recv_dv(Packet& packet)
{
    assert(packet.type == DV);
    assert(packet.dest == rp_.router_id_);

    auto it = dvs_.find(packet.src);
    if (it == dvs_.end()) {
        auto stat = dvs_.emplace(packet.src, DV_entry{});
        it = stat.first;
    }

    auto& dv_entry = it->second;
    dv_entry.last_update = rp_.sys->time();

    // Whenever we receive a DV from a router, a check is scheduled after the
    // out time.
    Alarm* alarm = new Alarm(Alarm_type::DV_CHK);
    alarm->content.router_id = packet.src;
    rp_.sys->set_alarm(&rp_, DV_OUT_TIME, alarm);

    auto payload_size = packet.size - PACKET_HEADER_SIZE;
    assert(payload_size % 4 == 0);
    size_t n_reachable = payload_size / 4;
    auto payload = static_cast<uint16_t*>(packet.payload);
    auto& dv = dv_entry.dv;

    bool updated = false;

    if (n_reachable != dv.size()) {
        // In this case, the DV must be changed, and we just reconstruct it
        // from the packet from scratch.
        updated = true;
        dv.clear();
        dv.reserve(n_reachable);
        for (size_t i = 0; i < n_reachable; ++i) {
            dv.emplace_back(ntohs(payload[2 * i]), ntohs(payload[2 * i + 1]));
        }
    } else {
        for (size_t i = 0; i < n_reachable; ++i) {
            auto dest_id = ntohs(payload[2 * i]);
            auto cost = ntohs(payload[2 * i + 1]);
            if (dv[i].first != dest_id || dv[i].second != cost) {
                dv[i].first = dest_id;
                dv[i].second = cost;
                updated = true;
            }
        }
    }

    if (updated) {
        if (compute_dv()) {
            send_dv();
        }
    }

    return false;
}

void DV_router::send_dv()
{
    auto n_reachable = rp_.forward_table_.size();
    Packet_size packet_size = PACKET_HEADER_SIZE + n_reachable * 4;

    for (auto port_id : rp_.active_ports_) {
        auto router_id = rp_.port_stats_[port_id].router_id;
        auto packet
            = prepare_packet(DV, packet_size, rp_.router_id_, router_id);
        auto dest = static_cast<uint16_t*>(packet) + 4;
        for (const auto& i : rp_.forward_table_) {
            dest[0] = htons(i.first);
            // Reverse poison.
            dest[1] = htons(
                i.second.port_id == port_id ? INFINITY_COST : i.second.cost);
            dest += 2;
        }
        rp_.sys->send(port_id, packet, packet_size);
    }

    return;
}

void DV_router::chk_stat(Router_id router)
{
    auto it = dvs_.find(router);
    // a neighbour already removed.
    if (it == dvs_.end())
        return;

    auto& entry = it->second;
    Time curr_time = rp_.sys->time();
    Time elapsed_time = curr_time - entry.last_update;
    if (elapsed_time >= DV_OUT_TIME) {
        dvs_.erase(it);
        if (compute_dv()) {
            send_dv();
        }
    }
    return;
}

bool DV_router::compute_dv()
{
    auto table = Forward_table{};

    auto update_entry
        = [&](Forward& entry, Port_id port_id, Router_id router_id, Time cost) {
              if (cost < entry.cost || entry.port_id == -1) {
                  entry.port_id = port_id;
                  entry.router_id = router_id;
                  entry.cost = cost;
              }
          };

    for (auto port_id : rp_.active_ports_) {
        auto& port_stat = rp_.port_stats_[port_id];
        assert(port_stat.if_conn);

        auto router_id = port_stat.router_id;
        // Forward table entry to the neighbour.
        auto& entry = table[router_id];
        update_entry(entry, port_id, router_id, port_stat.rtt);

        auto it = dvs_.find(router_id);
        if (it == dvs_.end()) {
            // Neighbour with its DV not received yet or removed due to timeout
            // for the heartbeat, unable to route other packets through it.
            continue;
        }
        auto& dv = it->second.dv;
        for (const auto& i : dv) {
            // Skip circular routing to itself.
            if (i.first == rp_.router_id_)
                continue;
            auto& entry = table[i.first];
            Time cost = port_stat.rtt <= INFINITY_COST - i.second
                ? port_stat.rtt + i.second
                : INFINITY_COST;
            update_entry(entry, port_id, router_id, cost);
        }
    }

    if (table == rp_.forward_table_) {
        return false;
    } else {
        rp_.forward_table_.swap(table);
        return true;
    }
}

const Time DV_router::DV_SEND_INTERV = 30 * 1000;
const Time DV_router::DV_OUT_TIME = 45 * 1000;

//
// LS routing method
//

void LS_router::update_port(
    Port_id port_id, Router_id router_id, Time rtt, Port_event event)
{
    // Compute edges of oneself from scratch.
    //
    // TODO: Make incremental update according to the actual event.
    self_edges_.clear();
    for (auto port_id : rp_.active_ports_) {
        const auto& stat = rp_.port_stats_[port_id];
        assert(stat.if_conn);
        self_edges_.emplace_back(stat.router_id, stat.rtt);
    }
    std::sort(self_edges_.begin(), self_edges_.end());

    compute_forward();
    bcast_ls();
    return;
}

bool LS_router::recv_ls(Packet& packet)
{
    if (packet.src == rp_.router_id_) {
        return false;
    }

    auto curr_time = rp_.sys->time();
    auto& stat = routers_[packet.src];

    // Always update last update time and schedule a check of its status
    // whenever we hear anything from the router.
    stat.last_update = curr_time;
    Alarm* alarm = new Alarm(Alarm_type::LS_CHK);
    alarm->content.router_id = packet.src;
    rp_.sys->set_alarm(&rp_, LS_router::LS_OUT_TIME, alarm);

    auto seq = ntohl(*(static_cast<Seq*>(packet.payload)));
    if (seq < stat.avail_seq) {
        return false;
    }
    stat.avail_seq = seq + 1;

    size_t entries_size = packet.size - PACKET_HEADER_SIZE - 4;
    assert(entries_size % 4 == 0);
    auto n_entries = entries_size / 4;

    // Parse the content.
    auto content = static_cast<const uint16_t*>(packet.payload) + 2;
    Edges edges{};
    for (size_t i = 0; i < n_entries; ++i) {
        edges.emplace_back(ntohs(content[0]), ntohs(content[1]));
        content += 2;
    }

    // It needs to be forwarded whether or not it is new.  It could be a
    // heartbeat.
    bcast_ls(packet.src, edges, seq);

    auto& curr_edges = stat.edges;
    if (edges != curr_edges) {
        curr_edges.swap(edges);
        compute_forward();
    }

    return false;
}

void LS_router::chk_stat(Router_id router)
{
    auto it = routers_.find(router);
    if (it == routers_.end()) {
        // Stale routers already removed.
        return;
    }

    Time curr_time = rp_.sys->time();
    auto elapsed = curr_time - it->second.last_update;
    if (elapsed >= LS_OUT_TIME) {
        routers_.erase(it);
        compute_forward();
    }
    return;
}

void LS_router::bcast_ls()
{
    auto seq = next_seq_;
    ++next_seq_;
    bcast_ls(rp_.router_id_, self_edges_, seq);
    return;
}

void LS_router::bcast_ls(Router_id router_id, const Edges& edges, Seq seq)
{
    auto packet_size = PACKET_HEADER_SIZE + 4 * (edges.size() + 1);

    for (auto port_id : rp_.active_ports_) {
        assert(rp_.port_stats_[port_id].if_conn);

        auto packet = prepare_packet(LS, packet_size, router_id, 0);

        *(static_cast<Seq*>(packet) + 2) = htonl(seq);
        auto dest = static_cast<uint16_t*>(packet) + 6;

        for (const auto& i : edges) {
            dest[0] = htons(i.first);
            dest[1] = htons(i.second);
            dest += 2;
        }

        rp_.sys->send(port_id, packet, packet_size);
    }
    return;
}

const Time LS_router::LS_BCAST_INTERV = 30 * 1000;
const Time LS_router::LS_OUT_TIME = 45 * 1000;

//
// Core Dijkstra for LS routing.
//

void LS_router::compute_forward()
{
    auto& table = rp_.forward_table_;
    const auto& port_stats = rp_.port_stats_;

    // Compute forward table from scratch.
    //
    // TODO: Make it more incremental from the actual changed happened.
    table.clear();

    std::priority_queue<Relax> queue{};
    for (auto port_id : rp_.active_ports_) {
        const auto& stat = port_stats[port_id];
        assert(stat.if_conn);
        queue.emplace(Relax{ stat.router_id, stat.rtt, port_id });
    }

    while (!queue.empty()) {
        const auto& relax = queue.top();
        bool if_proc = relax.dest != rp_.router_id_
            && (table.count(relax.dest) == 0
                   || table[relax.dest].cost > relax.cost);
        if (if_proc) {
            auto& forward = table[relax.dest];
            forward.router_id = port_stats[relax.next].router_id;
            forward.port_id = relax.next;
            forward.cost = relax.cost;

            auto it = routers_.find(relax.dest);
            if (it != routers_.end()) {
                const auto& edges = it->second.edges;
                auto curr_next = relax.next;
                auto curr_cost = relax.cost;
                for (const auto& i : edges) {
                    queue.emplace(
                        Relax{ i.first, curr_cost + i.second, curr_next });
                }
            }
        }
        queue.pop();
    }

    return;
}

//
// Routing protocol interface.
//
// Public functions.
//

RoutingProtocolImpl::RoutingProtocolImpl(Node* n)
    : RoutingProtocol(n)
{
    sys = n;
    // add your own code
}

RoutingProtocolImpl::~RoutingProtocolImpl()
{
    // add your own code (if needed)
}

void RoutingProtocolImpl::init(unsigned short num_ports,
    unsigned short router_id, eProtocolType protocol_type)
{
    // add your own code

    port_stats_.assign(num_ports, Port_stat());
    router_id_ = router_id;
    protocol_type_ = protocol_type;

    ping_ports();
    // Schedule the first port checking.  Later checking are scheduled after
    // their update.
    for (size_t i = 0; i < num_ports; ++i) {
        sched_port_chk(i);
    }

    if (protocol_type == P_DV) {
        dv_router_.reset(new DV_router(*this));
        // No need to make heartbeat right now because there is nothing yet.
        sched_dv_heartbeat();
    } else if (protocol_type == P_LS) {
        ls_router_.reset(new LS_router(*this));
        sched_ls_heartbeat();
    } else {
        assert(0);
    }
}

void RoutingProtocolImpl::handle_alarm(void* data)
{
    // add your own code
    auto alarm_ptr = static_cast<Alarm*>(data);
    Alarm& alarm = *alarm_ptr;

    switch (alarm.type) {
    case (Alarm_type::PORT_CHK):
        chk_port(alarm.content.port_id);
        break;
    case (Alarm_type::PING_REQ):
        ping_ports();
        break;
    case (Alarm_type::DV_CHK):
        assert(dv_router_ != nullptr);
        dv_router_->chk_stat(alarm.content.router_id);
        break;
    case (Alarm_type::DV_REQ):
        assert(dv_router_ != nullptr);
        dv_router_->send_dv();
        sched_dv_heartbeat();
        break;
    case (Alarm_type::LS_CHK):
        assert(ls_router_ != nullptr);
        ls_router_->chk_stat(alarm.content.router_id);
        break;
    case (Alarm_type::LS_REQ):
        assert(ls_router_ != nullptr);
        ls_router_->bcast_ls();
        sched_ls_heartbeat();
        break;
    default:
        assert(0);
    };

    delete alarm_ptr;
    return;
}

void RoutingProtocolImpl::recv(
    unsigned short port, void* packet, unsigned short size)
{
    // add your own code
    Packet p = parse_packet(port, packet, size);
    assert(p.size == size);

    bool owner_taken = false;

    switch (p.type) {
    case (DATA):
        owner_taken = recv_data(p);
        break;
    case (PING):
        owner_taken = recv_ping(p);
        break;
    case (PONG):
        owner_taken = recv_pong(p);
        break;
    case (DV):
        assert(dv_router_ != nullptr);
        owner_taken = dv_router_->recv_dv(p);
        break;
    case (LS):
        assert(ls_router_ != nullptr);
        owner_taken = ls_router_->recv_ls(p);
        break;
    default:
        assert(0);
    };

    if (!owner_taken) {
        free(packet);
    }

    return;
}

//
// Router interface internal functions.
//

void RoutingProtocolImpl::ping_ports()
{
    Packet_size size = PACKET_HEADER_SIZE + 4;
    for (Port_id port = 0; port < port_stats_.size(); ++port) {
        void* packet = prepare_packet(PING, size, router_id_, 0);
        uint32_t& time_slot = *(static_cast<uint32_t*>(packet) + 2);
        time_slot = htonl(sys->time());
        sys->send(port, packet, size);
    }

    Alarm* alarm = new Alarm(Alarm_type::PING_REQ);
    sys->set_alarm(this, PING_INTERV, alarm);
}

void RoutingProtocolImpl::chk_port(Port_id port)
{
    Time curr_time = sys->time();
    Time elapsed = curr_time - port_stats_[port].last_update;
    if (elapsed >= PING_OUT_TIME) {
        disassoc_port(port);
    }
    return;
}

void RoutingProtocolImpl::disassoc_port(Port_id port)
{
    Port_stat& stat = port_stats_[port];
    if (!stat.if_conn) {
        assert(active_ports_.count(port) == 0);
        return;
    }

    auto it = active_ports_.find(port);
    assert(it != active_ports_.end());
    active_ports_.erase(it);

    stat.if_conn = false;
    stat.router_id = 0;
    stat.rtt = INFINITY_COST;
    stat.last_update = sys->time();

    // Inform the individual routers only when all the states are correctly
    // updated.
    if (dv_router_ != nullptr) {
        dv_router_->update_port(port, 0, 0, Port_event::DISCONN);
    }
    if (ls_router_ != nullptr) {
        ls_router_->update_port(port, 0, 0, Port_event::DISCONN);
    }

    return;
}

void RoutingProtocolImpl::sched_port_chk(Port_id port)
{
    Alarm* alarm = new Alarm(Alarm_type::PORT_CHK);
    alarm->content.port_id = port;
    sys->set_alarm(this, PING_OUT_TIME, alarm);
    return;
}

bool RoutingProtocolImpl::recv_data(Packet& packet)
{
    assert(packet.type == DATA);

    if (packet.dest == router_id_) {
        return false;
    }

    auto entry = forward_table_.find(packet.dest);
    if (entry == forward_table_.end()) {
        // Unable to find the destination, drop packet.
        return false;
    }

    auto& forward = entry->second;
    sys->send(forward.port_id, packet.packet, packet.size);
    return true;
}

bool RoutingProtocolImpl::recv_ping(Packet& packet)
{
    assert(packet.type == PING);

    auto packet_8 = reinterpret_cast<uint8_t*>(packet.packet);
    auto packet_16 = reinterpret_cast<uint16_t*>(packet_8);

    *packet_8 = PONG;
    packet_16[2] = htons(router_id_);
    packet_16[3] = htons(packet.src);
    sys->send(packet.port, packet.packet, packet.size);

    return true;
}

bool RoutingProtocolImpl::recv_pong(Packet& packet)
{
    assert(packet.type == PONG);

    // Get current time as the first thing for better accuracy in RTT
    // estimation.
    Time curr_time = sys->time();

    sched_port_chk(packet.port);

    auto& stat = port_stats_[packet.port];

    // Preserve some previous states to get what actually changed (or not
    // changed).
    auto prev_conn = stat.if_conn;
    auto prev_router_id = stat.router_id;
    auto prev_rtt = stat.rtt;

    stat.if_conn = true;
    stat.router_id = packet.src;
    Time send_time = ntohl(*static_cast<Time*>(packet.payload));
    stat.rtt = curr_time - send_time;
    stat.last_update = curr_time;

    active_ports_.insert(packet.port);
    if (!prev_conn || prev_router_id != stat.router_id
        || prev_rtt != stat.rtt) {
        // If a new connection or anything changed.

        if (dv_router_ != nullptr) {
            dv_router_->update_port(
                packet.port, packet.src, stat.rtt, Port_event::CONN);
        }
        if (ls_router_ != nullptr) {
            ls_router_->update_port(
                packet.port, packet.src, stat.rtt, Port_event::CONN);
        }
    }

    return false;
}

const Time RoutingProtocolImpl::PING_INTERV = 10 * 1000;
const Time RoutingProtocolImpl::PING_OUT_TIME = 15 * 1000;

// add more of your own code
