cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 11)
project(RADSim)

set(CMAKE_BINARY_DIR "./build/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

SET(DESIGN "aximm_hello_world" CACHE STRING "Design directory to be compiled. Must be under rad-flow/rad-sim/example-designs")
message(STATUS "Compiling the ${DESIGN} design")

add_subdirectory(sim)
add_subdirectory(example-designs)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
