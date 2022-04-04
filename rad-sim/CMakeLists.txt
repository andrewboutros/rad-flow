cmake_minimum_required(VERSION 3.19)
set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-11")
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-11")
set(CMAKE_CXX_STANDARD 11)
project(RADSim)

message("${CMAKE_PREFIX_PATH}")
set(CMAKE_BINARY_DIR "./build/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(test)