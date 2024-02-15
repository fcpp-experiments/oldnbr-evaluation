// Copyright © 2024 Giorgio Audrito, Daniele Bortoluzzi, Giordano Scarso. All Rights Reserved.

/**
 * @file case-study.hpp
 * @brief Case study evaluate oldnbr operator.
 * 
 */

#ifndef CASE_STUDY_OLDNBR_H_
#define CASE_STUDY_OLDNBR_H_

#define BIG     0
#define SMALL   1

#include <string>
#include <ctime>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <cstring>
#include <vector>
#include <sstream>

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
    //! @brief Average of received alerts by current node.
    template <typename>
    struct avg_alert_per_node{};

    //! @brief Classic version of sp_collection algorithm
    struct classic {};
    //! @brief Oldnbr version of sp_collection algorithm
    struct oldnbr {};
    
    //! @brief The rating of the current node.
    struct node_rating {};
    //! @brief The alert counter of the current node.
    template <typename>
    struct node_alert_counter {};
    //! @brief The parent of the current node.
    struct node_parent {};
    //! @brief The rating of parent of the current node.
    struct node_rating_parent {};
    //! @brief A source of the network.
    struct node_source {};
    //! @brief The rating of the source.
    template <typename>
    struct source_alert_counter {};
}

namespace configurations {
    #if defined(AP_USE_CASE) && (AP_USE_CASE == AP_USE_CASE_BIG)
        //! @brief Number of people in the area.
        constexpr int node_num = 100;
        //! @brief The maximum communication range between nodes.
        constexpr size_t communication_range = 150;
    #elif defined(AP_USE_CASE) && (AP_USE_CASE == AP_USE_CASE_SMALL)
        //! @brief Number of people in the area.  
        constexpr int node_num = 10; 
        //! @brief The maximum communication range between nodes.
        constexpr size_t communication_range = 450;
    #else
        //! @brief Number of people in the area.  
        constexpr int node_num = 10; 
        //! @brief The maximum communication range between nodes.
        constexpr size_t communication_range = 450;
    #endif

    //! @brief Dimensionality of the space.
    constexpr size_t dim = 2;

    //! @brief End of simulated time.
    constexpr size_t end = 250;
}

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

//! @brief Compute rating using old and nbr communications with each neighbour.
FUN field<real_t> oldNbrConnection(ARGS) { CODE
    return oldnbr(CALL, field<real_t>{0.0}, [&](field<real_t> o, field<real_t> n){
        return make_tuple(
            n,
            mux(o == 0, n/2, o) + mod_other(CALL, 1, 0)
        );
    });
}

template <typename node_t, typename P, typename T, typename U, typename G, typename R, typename = common::if_signature<G, T(T,T)>>
T sp_collection_mod(ARGS, P const& distance, T const& value, U const& null, G&& accumulate, field<R> const& rating) { CODE
    tuple<T, R, device_t> result = nbr(CALL, make_tuple(T(null), (R)0, node.uid), [&](field<tuple<T, R, device_t>> x){

        auto best_neigh_field =  min_hood( CALL, make_tuple(nbr(CALL, distance), -rating, nbr_uid(CALL)) );
        R best_neigh_rating_computed = -get<1>(best_neigh_field);
        device_t best_neigh_computed = get<2>(best_neigh_field);

        R rating = get<1>(self(CALL, x));
        device_t parent = get<2>(self(CALL, x));
        T folded_value = fold_hood(CALL, accumulate, mux(get<2>(x) == node.uid, get<0>(x), (T)null), value);

        real_t rating_evolved = rating*0.8;

        if (best_neigh_computed != parent && best_neigh_rating_computed < rating_evolved) {
            return make_tuple(
                folded_value,
                rating_evolved,
                parent
            );
        } else {
            return make_tuple(
                folded_value,
                best_neigh_rating_computed,
                best_neigh_computed
            );
        }
    });
    T computed_value = get<0>(result);
    R computed_rating = get<1>(result);
    device_t computed_parent = get<2>(result);

    node.storage(fcpp::coordination::tags::node_parent{}) = computed_parent;
    node.storage(fcpp::coordination::tags::node_rating_parent{}) = computed_rating;

    return computed_value;
}

//! @brief Main function.
MAIN() {
    // import tag names in the local scope.
    using namespace tags;
    using namespace fcpp::component::tags;

    // usage of node storage
    node.storage(node_size{}) = 10;
    node.storage(node_color{}) = color(BLACK);
    node.storage(node_shape{}) = shape::sphere;

    /*****************/
    bool source = node.uid == 0; // source is node 0
    node.storage(node_source{}) = source;
    if (source) {
        node.storage(node_color{}) = color(GREEN);
        fcpp::common::get<sleep_ratio>(node.connector_data()) = 0.0; // the source never ends itself
        fcpp::common::get<send_power_ratio>(node.connector_data()) = 1.0; // the source has perfect "send" power ratio
        fcpp::common::get<recv_power_ratio>(node.connector_data()) = 1.0; // the source has perfect "recv" power ratio
    }

    auto adder = [](real_t x, real_t y) {
        return x+y;
    };

    real_t distance = coordination::abf_distance(CALL, source);
    field<real_t> rating = oldNbrConnection(CALL); 

    // std::cout << node.uid << std::endl;

    real_t value = coordination::sp_collection_mod(CALL, distance, 1.0, 0, adder, rating);

    node.storage(node_alert_counter<tags::oldnbr>{})    = value;
    node.storage(node_rating{})                         = rating;
    node.storage(sleep_ratio{})                         = fcpp::common::get<sleep_ratio>(node.connector_data());

    // update counter of the source
    if (node.storage(fcpp::coordination::tags::node_source{})) {
        node.storage(fcpp::coordination::tags::source_alert_counter<tags::oldnbr>{}) = value;
    }

    std::cout << std::endl;

    /*****************/
}
//! @brief Export types used by the main function (update it when expanding the program).
FUN_EXPORT main_t = export_list<double, 
                                int,    
                                field<int>, 
                                field<real_t>, 
                                tuple<device_t, real_t>,
                                tuple<real_t, real_t, device_t>,
                                tuple<real_t, int, device_t>,
                                field<tuple<double, real_t, device_t>>,
                                field<tuple<double, int, device_t>>,
                                tuple<real_t, real_t, device_t>,
                                sp_collection_t<real_t, real_t>,
                                abf_distance_t
                                >;

} // namespace coordination

// [SYSTEM SETUP]

//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;
//! @brief Import tags used by configurations.
using namespace coordination::configurations;

//! @brief Description of the round schedule.
using round_s = sequence::periodic<
    distribution::interval_n<times_t, 0, 1>,    // uniform time in the [0,1] interval for start
    distribution::weibull_n<times_t, 10, 1, 10>, // weibull-distributed time for interval (10/10=1 mean, 1/10=0.1 deviation)
    distribution::constant_n<times_t, coordination::configurations::end+5>
>;
//! @brief The sequence of network snapshots (one every simulated second).
using log_s = sequence::periodic_n<1, 0, 1, coordination::configurations::end>;
//! @brief The sequence of node generation events (node_num devices all generated at time 0).
using spawn_s = sequence::multiple_n<node_num, 0>;
//! @brief The distribution of initial node positions (random in a 500x500 square).
using rectangle_d = distribution::rect_n<1, 0, 0, 500, 500>;
//! @brief Shorthand for an constant input distribution.
template <typename T, typename R = double>
using i = distribution::constant_i<R, T>;
//! @brief Shorthand for a constant numeric distribution.
template <intmax_t num, intmax_t den = 1>
using n = distribution::constant_n<double, num, den>;

//! @brief The contents of the node storage as tags and associated types.
using store_t = tuple_store<
    node_color,                     color,
    node_size,                      double,
    node_shape,                     shape,

    node_alert_counter<classic>,    real_t,
    node_alert_counter<oldnbr>,     real_t,
    node_rating,                    field<real_t>,
    node_parent,                    device_t,
    node_rating_parent,             real_t,
    node_source,                    bool,

    source_alert_counter<classic>,  real_t,
    source_alert_counter<oldnbr>,   real_t,
    
    sleep_ratio,                    real_t
>;
//! @brief The tags and corresponding aggregators to be logged (change as needed).
using aggregator_t = aggregators<
    node_size,                          aggregator::mean<double>,
    node_alert_counter<classic>,        aggregator::sum<real_t>,
    node_alert_counter<oldnbr>,         aggregator::sum<real_t>,
    source_alert_counter<classic>,      aggregator::sum<real_t>,
    source_alert_counter<oldnbr>,       aggregator::sum<real_t>
>;

template <typename T> using sum_source_alert_counter = aggregator::sum<source_alert_counter<T>>;

template <template<class> typename T, typename... Ts>
using lines_t = plot::join<plot::value<T<Ts>>...>;

using avg_alert_per_node_t = plot::split<plot::time, lines_t<avg_alert_per_node, classic, oldnbr>>;
using sum_source_alert_counter_t = plot::split<plot::time, lines_t<sum_source_alert_counter, classic, oldnbr>>;
using plot_t = plot::join<sum_source_alert_counter_t, avg_alert_per_node_t>;

//! @brief Connection predicate (supports power and sleep ratio, 50% loss at 70% of communication range)
using connect_t = connect::radial<70, connect::powered<coordination::configurations::communication_range, 1, dim>>;
//using connect_t = connect::fixed<100>;

//! @brief The general simulation options.
DECLARE_OPTIONS(list,
    parallel<false>,      // multithreading enabled on node rounds
    synchronised<false>, // optimise for asynchronous networks
    program<coordination::main>,   // program to be run (refers to MAIN above)
    exports<coordination::main_t>, // export type list (types used in messages)
    retain<metric::retain<5,1>>,   // messages are kept for 2 seconds before expiring
    round_schedule<round_s>, // the sequence generator for round events on nodes
    log_schedule<log_s>,     // the sequence generator for log events on the network
    spawn_schedule<spawn_s>, // the sequence generator of node creation events on the network
    store_t,       // the contents of the node storage
    aggregator_t,  // the tags and corresponding aggregators to be logged
    log_functors<
        avg_alert_per_node<classic>,    functor::div<aggregator::sum<node_alert_counter<classic>>, n<coordination::configurations::node_num>>,
        avg_alert_per_node<oldnbr>,     functor::div<aggregator::sum<node_alert_counter<oldnbr>>, n<coordination::configurations::node_num>>
    >,
    init<
        x,                  rectangle_d, // initialise position randomly in a rectangle for new nodes
        send_power_ratio,   distribution::interval_n<times_t, 4, 5, 5>, // greater is better
        recv_power_ratio,   distribution::interval_n<times_t, 5, 5, 5>, // greater is better
        sleep_ratio,        distribution::interval_n<times_t, 0, 1, 5> // less is better
    >,
    plot_type<plot_t>,
    dimension<dim>, // dimensionality of the space
    connector<connect_t>,  // the connection predicate
    shape_tag<node_shape>, // the shape of a node is read from this tag in the store
    size_tag<node_size>,   // the size  of a node is read from this tag in the store
    color_tag<node_color>  // the color of a node is read from this tag in the store
);

} // namespace option

} // namespace fcpp


#endif // CASE_STUDY_OLDNBR_H_
