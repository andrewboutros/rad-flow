cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 11)
project(RADSim)

set(CMAKE_BINARY_DIR "./build/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(SystemCLanguage ${PACKAGE_FIND_VERSION} QUIET CONFIG)

if(SystemCLanguage_FOUND)
  message(STATUS "Found SystemC ${SystemCLanguage_VERSION} via ${SystemCLanguage_CONFIG}")
  message(STATUS "SystemC dir: ${SystemCLanguage_DIR}")
else()
	message(FATAL_ERROR "Cannot find SystemC installed on your system. Please follow the setup instructions on the repo to install SystemC and try again!")
endif()

SET(DESIGN "mvm" CACHE STRING "Design directory to be compiled. Must be under rad-flow/rad-sim/example-designs")
message(STATUS "Compiling the ${DESIGN} design")

add_subdirectory(sim)
add_subdirectory(example-designs)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
