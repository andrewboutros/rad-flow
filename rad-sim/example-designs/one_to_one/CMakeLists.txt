cmake_minimum_required(VERSION 3.16)
find_package(SystemCLanguage CONFIG REQUIRED)

include_directories(
  ./
  modules
  ../../sim
  ../../sim/noc
  ../../sim/noc/booksim
  ../../sim/noc/booksim/networks
  ../../sim/noc/booksim/routers
  ../../sim/dram
  ../../sim/dram/DRAMsim3
  ../../sim/dram/DRAMsim3/src
  ../../sim/dram/DRAMsim3/ext/headers
)

set(srcfiles
  modules/traffic_generator.cpp
  one_to_one_top.cpp
  one_to_one_driver.cpp
  one_to_one_system.cpp
  ../../sim/dram/mem_controller.cpp
)

set(hdrfiles
  modules/traffic_generator.hpp
  one_to_one_top.hpp
  one_to_one_driver.hpp
  one_to_one_system.hpp
  ../../sim/dram/mem_controller.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design PUBLIC SystemC::systemc booksim noc)