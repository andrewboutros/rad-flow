cmake_minimum_required(VERSION 3.16)
find_package(SystemCLanguage CONFIG REQUIRED)

add_subdirectory(modules/rtl)

include_directories(
  ./
  modules
  ../../sim
  ../../sim/noc
  ../../sim/noc/booksim
  ../../sim/noc/booksim/networks
  ../../sim/noc/booksim/routers
)

include_directories(SYSTEM "${VERILATOR_ROOT}/include")

set(srcfiles
  modules/fifo.cpp
  modules/register_file.cpp
  modules/sim_utils.cpp
  modules/instructions.cpp
  modules/dispatcher.cpp
  modules/collector.cpp
  modules/datapath.cpp
  modules/pipeline.cpp
  modules/sysc_mvm.cpp
  modules/rtl_mvm.cpp
  modules/weight_loader.cpp
  modules/inst_loader.cpp
  mlp_top.cpp
  mlp_driver.cpp
  mlp_int8_system.cpp
)

set(hdrfiles
  modules/fifo.hpp
  modules/register_file.hpp
  modules/sim_utils.hpp
  modules/instructions.hpp
  modules/dispatcher.hpp
  modules/collector.hpp
  modules/datapath.hpp
  modules/pipeline.hpp
  modules/sysc_mvm.hpp
  modules/rtl_mvm.hpp
  modules/weight_loader.hpp
  modules/inst_loader.hpp
  modules/config.hpp
  mlp_top.hpp
  mlp_driver.hpp
  mlp_int8_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(mlp_int8 STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(mlp_int8 PUBLIC SystemC::systemc booksim noc rtl_designs)
