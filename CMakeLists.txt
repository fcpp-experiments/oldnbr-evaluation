cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
option(FCPP_INTERNAL_TESTS "Build internal tests for FCPP." OFF)
add_subdirectory(./fcpp/src)

fcpp_setup()

project(
    AP_Engine
    VERSION 0.0.0
    DESCRIPTION "Evaluation of \"oldnbr\" operator."
)

set(CMAKE_CXX_STANDARD 17)

# Runner.
fcpp_target("./run/graphic.cpp" ON)
fcpp_target("./run/batch.cpp" OFF)
