#include <cassert>
#include <cstdint>

#include <arpa/inet.h>

#include "RoutingProtocolImpl.h"
#include "global.h"

//
// Small utilities.
//

Packet parse_packet(Port_id port, void* packet, Packet_size size)
{
    Packet res;

    res.type = *static_cast<ePacketType*>(packet);
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
    uint8_t* packet = new uint8_t[size];
    packet[0] = type;
    packet[1] = 0;

    auto packet_16 = static_cast<uint16_t*>(packet);
    packet_16[1] = htons(size);
    packet_16[2] = htons(src);
    packet_16[3] = htons(dest);

    return static_cast<void*>(packet);
}

const size_t PACKET_HEADER_SIZE = 8;

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
    assert(packet.dest == router_id_);

    auto it = dvs_.find(packet.src);
    if (it == dvs_.end()) {
        auto stat = dvs_.emplace(packet.src, DV_entry{});
        it = stat.first;
    }

    auto& dv_entry = it->second;
    dv_entry.last_update = sys->time();
    Alarm* alarm = new Alarm(DV_CHK);
    alarm->content.router_id = packet.src;
    sys->set_alarm(this, DV_OUT_TIME, alarm);

    size_t n_reachable = (packet.size - PACKET_HEADER_SIZE) / 4;
    auto payload = static_cast<uint16_t*>(packet.payload);
    auto& dv = dv_entry.dv;
    // Update the entire DV.
    //
    // TODO: Make a diff comparison and skip local DV update if it is not
    // changed at all.
    dv.clear();
    dv.reserve(n_reachable);
    for (size_t i = 0; i < n_reachable; ++i) {
        dv.emplace_back(ntohs(payload[2 * i]), ntohs(payload[2 * i + 1]));
    }

    compute_dv();
    send_dv();
}

void DV_router::send_dv()
{
    auto n_reachable = forward_table_.size();
    Packet_size packet_size = PACKET_HEADER_SIZE + n_reachable * 4;

    for (auto port_id : active_ports_) {
        auto router_id = port_stats_[port_id].router_id;
        auto packet = prepare_packet(DV, packet_size, router_id_, router_id);
        auto dest = static_cast<uint16_t*>(packet) + 4;
        for (const auto& i : forward_table_) {
            dest[0] = htons(i.first);
            dest[1] = htons(
                i.second.port_id == port_id ? INFINITY_COST : i.second.cost);
            dest += 2;
        }
        sys->send(port_id, packet, packet_size);
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
    Time curr_time = sys->time();
    Time elapsed_time = curr_time - entry.last_update;
    if (elapsed_time >= DV_OUT_TIME) {
        dvs_.erase(it);
        compute_dv();
        send_dv();
    }
    return;
}

bool DV_router::compute_dv()
{
    // Construct forward table from scratch.
    //
    // TODO: Update the forward table incrementally and return false when the
    // forward table has not actually been changed.
    forward_table_.clear();

    for (auto port_id : active_ports_) {
        auto& port_stat = port_stats_[port_id];
        assert(port_stat.if_conn);

        auto router_id = port_stat.router_id;
        auto entry = forward_table_[neighb_id];
        if (port_stat.rtt <= entry.cost) {
            entry.port_id = port_id;
            entry.router_id = router_id;
            entry.cost = port_stat.rtt;
        }

        auto it = dvs_.find(router_id);
        if (it == dvs_.end()) {
            // Neighbour with its DV not received yet, unable to route other
            // packets through it.
            continue;
        }
        auto& dv = it->second.dv;
        for (const auto& i : dv) {
            auto& entry = forward_table_[i.first];
            Time cost = port_stat.rtt <= INFINITY_COST - i.second
                ? port_stat.rtt + i.second
                : INFINITY_COST;
            if (cost <= entry.cost) {
                entry.port_id = port_id;
                entry.router_id = router_id;
                entry.cost = cost;
            }
        }
    }

    return true;
}

static const DV_router::DV_SEND_INTERV = 30 * 1000;
static const DV_router::DV_OUT_TIME = 45 * 1000;

//
// LS routing method
//

void LS_router::update_port(
    Port_id port_id, Router_id router_id, Time rtt, Port_event even)
{
    return;
}

bool LS_router::recv_ls(Packet& packet) { return false; }

void LS_router::chk_stat(Router_id router) { return; }

void LS_router::bcast_ls() { return; }

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
    for (size_t i = 0; i < num_ports; ++i) {
        sched_port_chk(i);
    }

    if (protocol_type == P_DV) {
        dv_router_.reset(new DV_router(*this);
    } else if (protocol_type == P_LS) {
        ls_router_.reset(new LS_router(*this);
    } else {
        assert(0);
    }
}

void RoutingProtocolImpl::handle_alarm(void* data)
{
    // add your own code
    Alarm& alarm = *static_cast<Alarm*>(data);

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
        DV_router_->send_dv();
        break;
    case (Alarm_type::LS_CHK):
        assert(ls_router_ != nullptr);
        ls_router_->chk_stat(alarm.content.router_id);
        break;
    case (Alarm_type::LS_REQ):
        assert(ls_router_ != nullptr);
        ls_router_->bcast_ls();
        break;
    case default:
        assert(0);
    };

    delete data;
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
    case default:
        assert(0);
    };

    if (!owner_taken) {
        delete packet;
    }

    return;
}

//
// Router interface internal functions.
//

void RoutingProtocolImpl::ping_ports()
{
    Packet_size size = PACKET_HEADER_SIZE + 4;
    for (Port_id port = 0; i < port_stats_.size(); ++i) {
        void* packet = prepare_packet(PING, size, router_id_, 0);
        uint32_t& time_slot = *(static_cast<uint32_t>(packet) + 2);
        time_slot = htonl(sys->time());
        sys->send(p, packet, size);
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

void disassoc_port(Port_id port)
{
    Port_stat& stat = port_stats_[port];
    if (!stat.if_conn)
        return;

    auto it = active_ports_.find(port);
    assert(it != active_ports_.end());
    active_ports_.erase(it);

    if (dv_router_ != nullptr) {
        dv_router_->update_port(port, 0, 0, Port_event::DISCONN);
    }
    if (ls_router_ != nullptr) {
        ls_router_->update_port(port, 0, 0, Port_event::DISCONN);
    }

    stat.if_conn = false;
    stat.id = 0;
    stat.rtt = 0;
    stat.last_update = sys->time();
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
    sys->send(forward.port, packet.packet, packet.size);
    return true;
}

bool RoutingProtocolImpl::recv_ping(Packet& packet)
{
    assert(packet.type == PING);

    auto packet_8 = static_cast<uint8_t*>(packet.packet);
    auto packet_16 = static_cast<uint16_t*>(packet_8);

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

static Time RoutingProtocolImpl::PING_INTERV = 10 * 1000;
static Time RoutingProtocolImpl::PING_OUT_TIME = 15 * 1000;

// add more of your own code
