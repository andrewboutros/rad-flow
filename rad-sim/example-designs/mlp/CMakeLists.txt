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
)

set(srcfiles
  modules/mvm.cpp
  modules/fifo.cpp
  modules/register_file.cpp
  modules/sim_utils.cpp
  modules/instructions.cpp
  modules/dispatcher.cpp
  modules/collector.cpp
  mlp_top.cpp
  mlp_driver.cpp
  mlp_system.cpp
)

set(hdrfiles
  modules/mvm.hpp
  modules/fifo.hpp
  modules/register_file.hpp
  modules/sim_utils.hpp
  modules/instructions.hpp
  modules/dispatcher.hpp
  modules/collector.hpp
  mlp_top.hpp
  mlp_driver.hpp
  mlp_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(mlp STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(mlp PUBLIC SystemC::systemc booksim noc)
