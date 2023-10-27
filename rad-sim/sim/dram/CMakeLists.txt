cmake_minimum_required(VERSION 3.16)
find_package(SystemCLanguage CONFIG REQUIRED)

add_subdirectory(DRAMsim3)

include_directories(
  ./
  ../
  ${SystemCLanguage_DIR}/../src/
  ./DRAMsim3
  ./DRAMsim3/src
  ../noc
  ../noc/booksim
  ../noc/booksim/routers
  ../noc/booksim/allocators
  ../noc/booksim/arbiters
  ../noc/booksim/networks
  ../noc/booksim/power
)

file(GLOB srcfiles
  "dram_axis.cpp"
  "dram_test.cpp"
  "mem_controller.cpp"
  "mem_controller_test.cpp"
)

file(GLOB hdrfiles
  "dram_axis.hpp"
  "dram_test.hpp"
  "mem_controller.hpp"
  "mem_controller_test.hpp"
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(dram STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(dram SystemC::systemc dramsim3)