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
  modules/embedding_lookup.cpp
  modules/feature_interaction.cpp
  modules/custom_feature_interaction.cpp
  modules/sim_utils.cpp
  modules/afifo.cpp
  modules/register_file.cpp
  modules/mvm.cpp
  modules/fifo.cpp
  modules/instructions.cpp
  modules/collector.cpp
  modules/portal.cpp
  dlrm_top.cpp
  dlrm_driver.cpp
  dlrm_system.cpp
)

set(hdrfiles
  modules/embedding_lookup.hpp
  modules/feature_interaction.hpp
  modules/custom_feature_interaction.hpp
  modules/sim_utils.hpp
  modules/afifo.hpp
  modules/register_file.hpp
  modules/mvm.hpp
  modules/fifo.hpp
  modules/instructions.hpp
  modules/collector.hpp
  modules/portal.hpp
  modules/dlrm_defines.hpp
  dlrm_top.hpp
  dlrm_driver.hpp
  dlrm_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(dlrm STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(dlrm PUBLIC SystemC::systemc booksim noc dram)
