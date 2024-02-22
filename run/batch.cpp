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
    //! @brief Construct the plotter object.
    option::plot_t p;
    //! @brief The component type (batch simulator with given options).
    using comp_t = component::batch_simulator<option::list>;
    //! @brief The list of initialisation values to be used for simulations.
    auto init_list = batch::make_tagged_tuple_sequence(
        batch::arithmetic<option::seed>(0, 999, 1),      // 100 different random seeds
        // generate output file name for the run
        batch::stringify<option::output>("output/batch", "txt"),
        batch::constant<option::plotter>(&p) // reference to the plotter object
    );
    //! @brief Runs the given simulations.
    batch::run(comp_t{}, init_list);
    //! @brief Builds the resulting plots.
    std::cout << plot::file("batch", p.build());
    return 0;
}
