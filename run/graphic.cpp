// Copyright © 2024 Giorgio Audrito, Daniele Bortoluzzi, Giordano Scarso. All Rights Reserved.

/**
 * @file runner.cpp
 * @brief Runner of case study to evaluate oldnbr operator.
 */

// [INTRODUCTION]
//! Importing the FCPP library.
#include "lib/fcpp.hpp"

#include "lib/case-study.hpp"

using namespace fcpp;


//! @brief The main function.
int main() {

    fcpp::option::plot_t p;
    std::cout << "/*\n";

    {
        //! @brief The network object type (interactive simulator with given options).
        using net_t = component::interactive_simulator<option::list>::net;
        //! @brief The initialisation values (simulation name).
        auto init_v = common::make_tagged_tuple<option::name, option::plotter>(
            "Oldnbr Evaluation",
            &p
        );
        //! @brief Construct the network object.
        net_t network{init_v};
        //! @brief Run the simulation until exit.
        network.run();
    }

    std::cout << "*/\n";
    std::cout << plot::file("runner", p.build());
    return 0;
}
