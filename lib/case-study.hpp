// Copyright © 2024 Giorgio Audrito, Daniele Bortoluzzi, Giordano Scarso. All Rights Reserved.

/**
 * @file case-study.hpp
 * @brief Case study evaluate oldnbr operator.
 * 
 */

#ifndef CASE_STUDY_OLDNBR_H_
#define CASE_STUDY_OLDNBR_H_

#include <string>
#include <ctime>
#include <cmath>

#include "lib/fcpp.hpp"

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Dummy ordering between positions (allows positions to be used as secondary keys in ordered tuples).
template <size_t n>
bool operator<(vec<n> const&, vec<n> const&) {
    return false;
}

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief Color of the current node.
    struct node_color {};
    //! @brief Size of the current node.
    struct node_size {};
    //! @brief Shape of the current node.
    struct node_shape {};
    // ... add more as needed, here and in the tuple_store<...> option below

    //! @brief The areaLog value reaching the gateway from the anomaly.
    struct gateway_log {};
}

//! @brief The maximum communication range between nodes.
constexpr size_t communication_range = 100;

// [AGGREGATE PROGRAM]

//! @brief Counts the number of messages received by each neighbour.
FUN field<int> uniConnection(ARGS) { CODE
    return old(CALL, field<int>{0}, [&](field<int> o){
        return o + mod_other(CALL, 1, 0);
    });
}

//! @brief Counts the number of bidirectional communications with each neighbour.
FUN field<int> biConnection(ARGS) { CODE
    return nbr(CALL, field<int>{0}, [&](field<int> n){
        return n + mod_other(CALL, 1, 0);
    });
}

//! @brief Returns the number of hops to the closest device where `source` is true.
FUN double distanceTo(ARGS, bool source) { CODE
    return nbr(CALL, INF, [&](field<double> n){
        return mux(source, 0.0, min_hood(CALL, n, INF)+1.0);
    });
}

//! @brief Propagates value along ascending dist.
GEN(T) T broadcast(ARGS, double dist, T value) { CODE
    tuple<double,T> loc{dist, value};
    return get<1>(nbr(CALL, loc, [&](field<tuple<double,T>> n){
        return make_tuple(dist, get<1>(min_hood(CALL, n, loc)));
    }));
}

//! @brief Block C - converge-cast.
GEN(F, G) double C(ARGS, bool sink, double value, F&& accumulate, G&& divide) { CODE
    double dist = distanceTo(CALL, sink);
    return get<1>(nbr(CALL, make_tuple(dist,value), [&](field<tuple<double,double>> n){
        field<int> conn = mux(get<0>(n) < dist, biConnection(CALL), 0);
        field<double> send = conn / (double)max(sum_hood(CALL, conn, 0), 1);
        field<double> recv = nbr(CALL, send);
        return make_tuple(dist, fold_hood(CALL, accumulate, map_hood(divide, get<1>(n), recv), value));
    }));
}

//! @brief Returns the number of hops between the closests source and dest devices.
FUN int distance(ARGS, bool source, bool dest) { CODE
    return broadcast(CALL, distanceTo(CALL, source), distanceTo(CALL, dest));
}

//! @brief Returns true on the way between the closest source and dest devices, false elsewhere
FUN bool channel(ARGS, bool source, bool dest, double width) { CODE
    return distanceTo(CALL, source) + distanceTo(CALL, dest) <= distance(CALL, source, dest) + width;
}

//! @brief Broadcasts the value in source along the channel between source and dest
GEN(T) T broadcastChannel(ARGS, bool source, bool dest, double width, T value) { CODE
    return channel(CALL, source, dest, width) ? broadcast(CALL, distanceTo(CALL, source), value) : value;
}


//! @brief Main function.
MAIN() {
    // import tag names in the local scope.
    using namespace tags;

    // sample code below (substitute with the solution to the exercises)...

    // usage of aggregate constructs
    field<double> f = nbr(CALL, 4.2); // nbr with single value
    int x = old(CALL, 0, [&](int a){  // old with initial value and update function
        return a+1;
    });
    int y = nbr(CALL, 0, [&](field<int> a){ // nbr with initial value and update function
        return min_hood(CALL, a);
    });

    // usage of node storage
    node.storage(node_size{}) = 10;
    node.storage(node_color{}) = color(GREEN);
    node.storage(node_shape{}) = shape::sphere;

    /*****************/
    bool gateway = node.uid == 0; // gateway is node 0
    // anomaly is in node 1 between times 25 and 50
    bool anomaly = node.uid == 1 and 25 < node.current_time() and node.current_time() < 50;
    double log = 1; // dummy log value (areaLog counts the area size)

    double areaLog = distanceTo(CALL, anomaly) < 10 ? C(CALL, anomaly, log, [](double x, double y){
        return x+y;
    }, [](double x, double y){
        return x*y;
    }) : 0;
    double result = broadcastChannel(CALL, anomaly, gateway, 10, areaLog);
    node.storage(tags::gateway_log{}) = gateway ? result : 0;
    if (gateway) {
        node.storage(node_color{}) = color(RED);
    }
    if (anomaly) {
        node.storage(node_color{}) = color(YELLOW);
    }

    /*****************/
}
//! @brief Export types used by the main function (update it when expanding the program).
FUN_EXPORT main_t = export_list<double, 
                                int,    
                                field<int>, 
                                field<double>, 
                                tuple<double,double>, 
                                tuple<field<double>,double>
                                >;

} // namespace coordination

// [SYSTEM SETUP]

//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;

//! @brief Number of people in the area.
constexpr int node_num = 100;
//! @brief Dimensionality of the space.
constexpr size_t dim = 2;

//! @brief Description of the round schedule.
using round_s = sequence::periodic<
    distribution::interval_n<times_t, 0, 1>,    // uniform time in the [0,1] interval for start
    distribution::weibull_n<times_t, 10, 1, 10> // weibull-distributed time for interval (10/10=1 mean, 1/10=0.1 deviation)
>;
//! @brief The sequence of network snapshots (one every simulated second).
using log_s = sequence::periodic_n<1, 0, 1>;
//! @brief The sequence of node generation events (node_num devices all generated at time 0).
using spawn_s = sequence::multiple_n<node_num, 0>;
//! @brief The distribution of initial node positions (random in a 500x500 square).
using rectangle_d = distribution::rect_n<1, 0, 0, 500, 500>;
//! @brief The contents of the node storage as tags and associated types.
using store_t = tuple_store<
    node_color,                 color,
    node_size,                  double,
    node_shape,                 shape,
    gateway_log,                double
>;
//! @brief The tags and corresponding aggregators to be logged (change as needed).
using aggregator_t = aggregators<
    node_size,                  aggregator::mean<double>
>;
//! @brief Connection predicate (supports power and sleep ratio, 50% loss at 70% of communication range)
using connect_t = connect::radial<70, connect::powered<coordination::communication_range, 1, dim>>;
//using connect_t = connect::fixed<100>;

//! @brief The general simulation options.
DECLARE_OPTIONS(list,
    parallel<true>,      // multithreading enabled on node rounds
    synchronised<false>, // optimise for asynchronous networks
    program<coordination::main>,   // program to be run (refers to MAIN above)
    exports<coordination::main_t>, // export type list (types used in messages)
    retain<metric::retain<5,1>>,   // messages are kept for 2 seconds before expiring
    round_schedule<round_s>, // the sequence generator for round events on nodes
    log_schedule<log_s>,     // the sequence generator for log events on the network
    spawn_schedule<spawn_s>, // the sequence generator of node creation events on the network
    store_t,       // the contents of the node storage
    aggregator_t,  // the tags and corresponding aggregators to be logged
    init<
        x,                  rectangle_d, // initialise position randomly in a rectangle for new nodes
        send_power_ratio,   distribution::constant_n<real_t,1,1>,
        recv_power_ratio,   distribution::constant_n<real_t,1,1>
    >,
    dimension<dim>, // dimensionality of the space
    connector<connect_t>,  // the connection predicate
    shape_tag<node_shape>, // the shape of a node is read from this tag in the store
    size_tag<node_size>,   // the size  of a node is read from this tag in the store
    color_tag<node_color>  // the color of a node is read from this tag in the store
);

} // namespace option

} // namespace fcpp


#endif // CASE_STUDY_OLDNBR_H_
