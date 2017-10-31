#ifndef ROUTINGPROTOCOLIMPL_H
#define ROUTINGPROTOCOLIMPL_H

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Node.h"
#include "RoutingProtocol.h"
#include "global.h"

//
// General small utilities.
//

/** The arithmetic time type as used by the system.
 */

using Time = unsigned int;

/** IDs for each of the routers.
 */

using Router_id = unsigned short;

/** IDs for the ports.
 *
 * This type is only utilized on the interface level.  In the core, ports are
 * better handled as size_t for naive indexing of the ports vector.
 */

using Port_id = unsigned short;

/** Sizes of the packets.
 */

using Packet_size = unsigned short;

//
// Port and neighbour handling.
//

/** The status of a port.
 *
 * This open struct relies more on direct manipulation inside the routing
 * protocol.  Not many methods are given.
 */

struct Port_stat {
    bool if_conn;
    Router_id router_id;
    Time rtt;
    Time last_update;

    Port_stat()
        : if_conn(false)
        , router_id(0)
        , rtt(0)
        , last_update(0)
    {
    }
};

/** Statues of all the ports.
 */

using Port_stats = std::vector<Port_stat>;

/** The set of active ports of a router.
 *
 * The contents are the indices to the port status vector.
 */

using Active_ports = std::set<size_t>;

//
// Core routing data structures.
//

/** Single entry inside an forward table.
 *
 * This gives the routing for any given router inside the forwarding table.
 */

struct Forward {
    Router_id router_id;
    Port_id port_id;
    Time cost;

    Forward()
        : router_id(-1)
        , port_id(-1)
        , cost(INFINITY_COST)
    {
    }
};

/** The main forwarding table.
 *
 * This is going to be used by both routing algorithms.  It is also used as the
 * DV for the router itself in DV routing.
 *
 * It is set as an map so that its entries could be serialized in a very
 * deterministic manner for the ease of comparison.
 */

using Forward_table = std::map<Router_id, Forward>;

//
// Events related: messages and alarms.
//

/** The content of an incoming packet.
 *
 * Note that this struct just serve to hold the parsed content and does *not*
 * take the ownership of the packet.
 */

struct Packet {
    ePacketType type;
    Port_id port;
    Packet_size size;
    Router_id src;
    Router_id dest;
    void* payload;
    void* packet;
};

/** Parse the given packet.
 */

Packet parse_packet(Port_id, void*, Packet_size);

/** Prepare a packet to send.
 *
 * New memory is allocated and the ownership is returned.
 */

void* prepare_packet(
    ePacketType type, Packet_size size, Router_id src, Router_id dest);

/** Size of the header of a packet.
 */

extern const size_t PACKET_HEADER_SIZE;

/** Types for different kinds of alarms.
 *
 * CHK types are for checking the activity of a port, DV entry, or an LS entry
 * to ensure that it is still alive.  The REQ types are generally for periodic
 * heartbeat actions to be performed.
 */

enum class Alarm_type { PORT_CHK, PING_REQ, DV_CHK, DV_REQ, LS_CHK, LS_REQ };

/** The actual alarms.
 *
 * In the current implementation, alarm will always be allocated by the sender
 * on the heap and released by the handler.
 */

struct Alarm {
    Alarm_type type;

    union {
        /** Port index.
         *
         * For PORT_CHK.
         */
        size_t port_id;

        /** Router ID.
         *
         * For DV_CHK and LS_CHK.
         */
        Router_id router_id;

    } content;

    Alarm(Alarm_type type)
        : type(type)
    {
    }
};

//
// The two core main routing protocol implementations.
//

class RoutingProtocolImpl;

/** Changes possible on a port.
 *
 * CONN - New connection or updated RTT.
 * DISCONN - Disconnection
 */

enum class Port_event { CONN, DISCONN };

/** The distance-vector router.
 */

class DV_router {
public:
    friend class RoutingProtocolImpl;

    DV_router(RoutingProtocolImpl& rp)
        : rp_{ rp }
        , dvs_{}
    {
    }

    /** Handle new port event.
     */

    void update_port(
        Port_id port_id, Router_id router_id, Time rtt, Port_event event);

    /** Handle new dv received.
     */

    bool recv_dv(Packet&);

    /** Send DV to its neighbours.
     *
     * Reverse poison is implemented so that the DV sent may not be the same as
     * the local forward table.
     */

    void send_dv();

    /** Check the status of a router.
     */

    void chk_stat(Router_id router);

private:
    /** Compute the distance vector.
     *
     * The return value indicates if there are changes happened.
     */

    bool compute_dv();

    static const Time DV_SEND_INTERV;
    static const Time DV_OUT_TIME;

    RoutingProtocolImpl& rp_;

    /** Distance vector from other routers.
     */

    using DV_t = std::vector<std::pair<Router_id, Time>>;

    struct DV_entry {
        DV_t dv;
        Time last_update;
    };

    using DVs = unordered_map<Router_id, DV_entry>;

    DVs dvs_;
};

/** The link-state router.
 */

class LS_router {
public:
    friend class RoutingProtocolImpl;

    LS_router(RoutingProtocolImpl& rp)
        : rp_{ rp }
    {
    }

    /** Handle a new port event.
     */

    void update_port(
        Port_id port_id, Router_id router_id, Time rtt, Port_event event);

    /** Handle an incoming LS packet.
     */

    bool recv_ls(Packet&);

    /** Check the status of a fellow router.
     */

    void chk_stat(Router_id);

    /** Broadcast your own link status.
     */

    void bcast_ls();

private:
    static const Time LS_BCAST_INTERV;
    static const Time LS_OUT_TIME;

    RoutingProtocolImpl& rp_;
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
     *
     * This method sends a ping to all the ports and schedule the next
     * heartbeat ping as well.
     */

    void ping_ports();

    /** Check if the given port is still alive.
     */

    void chk_port(Port_id port);

    /** Disassociate a port with its peer.
     *
     * The given port does not have to be connected to a peer already.  When it
     * is connected, the connection will be removed.
     */

    void disassoc_port(Port_id port);

    /** Schedule a check on the given port.
     *
     * The check is scheduled after the timeout cut-off has been exceeded.
     */

    void sched_port_chk(Port_id port);

    /** Handle incoming data packet.
     */

    bool recv_data(Packet&);

    /** Handle incoming ping packet.
     */

    bool recv_ping(Packet&);

    /** Handle incoming pong packet.
     */

    bool recv_pong(Packet&);

    /** Schedule a heartbeat for DV router.
     */

    inline void sched_dv_heartbeat()
    {
        sys->set_alarm(
            this, DV_router::DV_SEND_INTERV, new Alarm(Alarm_type::DV_REQ));
        return;
    }

    /** Schedule a heartbeat for LS router.
     */

    inline void sched_ls_heartbeat()
    {
        sys->set_alarm(
            this, LS_router::LS_BCAST_INTERV, new Alarm(Alarm_type::LS_REQ));
        return;
    }

    static const Time PING_INTERV;
    static const Time PING_OUT_TIME;

    Node* sys; // To store Node object; used to access GSR9999 interfaces

    Router_id router_id_;
    eProtocolType protocol_type_;

    Port_stats port_stats_{};
    Active_ports active_ports_{};
    Forward_table forward_table_{};

    friend class DV_router;
    friend class LS_router;
    std::unique_ptr<DV_router> dv_router_{};
    std::unique_ptr<LS_router> ls_router_{};
};

#endif
