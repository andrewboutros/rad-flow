cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)

include_directories(
  ./ext/fmt/include/fmt
  ./ext/headers
  ./src
  ${SystemCLanguage_DIR}/../src/
)

add_library(inih INTERFACE)
target_include_directories(inih INTERFACE ext/headers)

add_library(format INTERFACE)
target_include_directories(format INTERFACE ext/fmt/include)
target_compile_definitions(format INTERFACE FMT_HEADER_ONLY=1)

add_library(args INTERFACE)
target_include_directories(args INTERFACE ext/headers)

add_library(json INTERFACE)
target_include_directories(json INTERFACE ext/headers)

file(GLOB srcfiles
        "src/bankstate.cc"
        "src/channel_state.cc"
        "src/command_queue.cc"
        "src/common.cc"
        "src/configuration.cc"
        "src/controller.cc"
        "src/dram_system.cc"
        "src/hmc.cc"
        "src/refresh.cc"
        "src/simple_stats.cc"
        "src/timing.cc"
        "src/memory_system.cc")

file(GLOB hdrfiles
        "ext/fmt/include/fmt/*.h"
        "ext/headers/*.hpp"
        "ext/headers/*.h"
        "ext/headers/*.hxx"
        "src/bankstate.h"
        "src/channel_state.h"
        "src/command_queue.h"
        "src/common.h"
        "src/configuration.h"
        "src/controller.h"
        "src/dram_system.h"
        "src/hmc.h"
        "src/refresh.h"
        "src/simple_stats.h"
        "src/timing.h"
        "src/memory_system.h")

add_library(dramsim3 STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(dramsim3 PUBLIC inih format args json)