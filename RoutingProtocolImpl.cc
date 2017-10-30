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
