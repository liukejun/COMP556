#ifndef ROUTINGPROTOCOLIMPL_H
#define ROUTINGPROTOCOLIMPL_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "RoutingProtocol.h"
#include "global.h"

//
// General small utilities.
//

/** The arithmetic time type as used by the system.
 */

using Time = unsigned int;

/** The ID for each of the routers.
 */

using Router_id = unsigned short;

/** The ID for the ports.
 *
 * This type is only utilized on the interface level.  In the core, ports are
 * better handled as size_t for naive indexing the ports vector.
 */

using Port_id = unsigned short;

/** The size of the packets.
 */

using Packet_size = unsigned short;

//
// Port and neighbour handling.
//

/** The status of a port.
 *
 * This open struct relies on direct manipulation inside the routing protocol.
 */

struct Port_stat {
    bool if_conn;
    Router_id id;
    Time rtt;
    Time last_update;

    Port_stat()
        : if_conn(false)
        , id(0)
        , rtt(0)
        , last_update(0)
    {
    }
};

/** Statues of all the ports.
 */

using Port_stats = std::vector<Port_stat>;

/** The set of neighbours of a router.
 *
 * The contents are the indices to the port status vector.
 */

using Neighbs = std::unordered_set<size_t>;

//
// Core routing data structures.
//

/** Single entry inside an forward table.
 */

struct Forward_entry {
    Router_id next;
    Time cost;
};

/** The main forwarding table.
 *
 * This is going to be used by both routing algorithms.  It is also used as the
 * DV for the router itself in DV routing.
 */

using Forward_table = std::unordered_map<Router_id, Forward_entry>;

//
// Events related: messages and alarms.
//

/** Get the type of a packet.
 */

ePacketType get_packet_type(void*);

/** Types for different kinds of alarms.
 */

enum class Alarm_type {
    PING_TIMEOUT,
    PING_REQ,
    DV_TIMEOUT,
    DV_REQ,
    LS_TIMEOUT,
    LS_REQ
};

/** The actual alarms.
 *
 * In the current implementation, alarm will be allocated by the sender and
 * released by the receiver.
 */

struct Alarm {
    Alarm_type alarm_type;

    union {
        /** Port index.
         *
         * For PING_TIMEOUT.
         */
        size_t port_id;

        /** Router ID.
         *
         * For DV_TIMEOUT and LS_TIMEOUT.
         */
        Router_id router_id;

    } content;
};

//
// The two core main routing protocol implementations.
//

class RoutingProtocolImpl;

/** The distance-vector router.
 */

class DV_router {
public:
    DV_router(RoutingProtocolImpl&);

    void update_neighb(Router_id id, Time rtt);
    void remove_neighb(Router_id id, Time rtt);

    bool recv_dv(Port_id, void*, Packet_size);

private:
    /** Distance vector from other routers.
     */

    using DV = std::vector<std::pair<Router_id, Time>>;

    struct DV_entry {
        DV dv;
        Time last_update;
    };

    using DVs = unordered_map<Router_id, DV_entry>;

    DVs dvs_;
};

/** The link-state router.
 */

class LS_router {
public:
    LS_router(RoutingProtocolImpl&);
    void update_neighb(Router_id id, Time rtt);
    void remove_neighb(Router_id id, Time rtt);
    bool recv_ls(Port_id, void*, Packet_size);
};

/** Implementation of the routing protocol interface.
 */

class RoutingProtocolImpl : public RoutingProtocol {
public:
    RoutingProtocolImpl(Node* n);
    ~RoutingProtocolImpl();

    /** Initialize the routing protocol.
     *
     * As discussed in the assignment document, your RoutingProtocolImpl is
     * first initialized with the total number of ports on the router, the
     * router's ID, and the protocol type (P_DV or P_LS) that should be used.
     * See global.h for definitions of constants P_DV and P_LS.
     */

    void init(unsigned short num_ports, unsigned short router_id,
        eProtocolType protocol_type);

    /** Handle the given alarm.
     *
     * As discussed in the assignment document, when an alarm scheduled by your
     * RoutingProtoclImpl fires, your RoutingProtocolImpl's handle_alarm()
     * function will be called, with the original piece of "data" memory
     * supplied to set_alarm() provided. After you handle an alarm, the memory
     * pointed to by "data" is under your ownership and you should free it if
     * appropriate.
     */

    void handle_alarm(void* data);

    /** Receive the given packet.
     *
     * When a packet is received, your recv() function will be called with the
     * port number on which the packet arrives from, the pointer to the packet
     * memory, and the size of the packet in bytes. When you receive a packet,
     * the packet memory is under your ownership and you should free it if
     * appropriate. When a DATA packet is created at a router by the simulator,
     * your recv() function will be called for such DATA packet, but with a
     * special port number of SPECIAL_PORT (see global.h) to indicate that the
     * packet is generated locally and not received from a neighbor router.
     */

    void recv(unsigned short port, void* packet, unsigned short size);

private:
    /** Ping all ports of the router.
     */

    void ping_ports();

    /** Check if the given port is still alive.
     */

    void check_port(Port_id port);

    /** Handle incoming data packet.
     */

    bool recv_data(Port_id, void*, Packet_size);

    /** Handle incoming ping packet.
     */

    bool recv_ping(Port_id, void*, Packet_size);

    /** Handle incoming pong packet.
     */

    bool recv_pong(Port_id, void*, Packet_size);

    Node* sys; // To store Node object; used to access GSR9999 interfaces

    unsigned short router_id_;
    eProtocolType protocol_type_;

    Port_stats port_stats_;
    Forward_table forward_table_;

    friend class DV_router;
    friend class LS_router;
    std::unique_ptr<DV_router> dv_router_;
    std::unique_ptr<LS_router> ls_router_;
};

#endif
