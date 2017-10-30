#include <cassert>
#include <cstdint>

#include <arpa/inet.h>

#include "RoutingProtocolImpl.h"
#include "global.h"

//
// Small utilities.
//

Packet parse_packet(Port_id port, void* packet)
{
    Packet res;

    res.type = *static_cast<ePacketType*>(packet);
    res.port = port;

    auto packet_16 = static_cast<uint16_t*>(packet);
    res.size = ntohs(packet_16[1]);
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

void DV_router::update_neighb(
    Port_id port_id, Router_id router_id, Time rtt, Neighb_delta delta)
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

void LS_router::update_neighb(
    Port_id port_id, Router_id router_id, Time rtt, Neighb_delta delta)
{
    return;
}

bool LS_router::recv_ls(Packet& packet) { return false; }

void LS_router::chk_stat(Router_id router) { return; }

void LS_router::bcast_ls() { return; }


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
}

void RoutingProtocolImpl::handle_alarm(void* data)
{
    // add your own code
}

void RoutingProtocolImpl::recv(
    unsigned short port, void* packet, unsigned short size)
{
    // add your own code
}

// add more of your own code
