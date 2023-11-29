cmake_minimum_required(VERSION 3.19)
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


# modules/traffic_gen.cpp
set(srcfiles
    modules/embedding_lookup.cpp
    modules/black_box.cpp
    modules/feature_interaction.cpp
    modules/custom_feature_interaction.cpp
    modules/sim_utils.cpp
    modules/afifo.cpp
    modules/register_file.cpp
    modules/mvm.cpp
    modules/fifo.cpp
    modules/instructions.cpp
    modules/collector.cpp
    hbm_traffic_top.cpp
    hbm_traffic_driver.cpp
    hbm_traffic_system.cpp
)

# modules/traffic_gen.hpp
set(hdrfiles
    modules/embedding_lookup.hpp
    modules/feature_interaction.hpp
    modules/custom_feature_interaction.hpp
    modules/black_box.hpp
    modules/sim_utils.hpp
    modules/afifo.hpp
    modules/register_file.hpp
    modules/mvm.hpp
    modules/fifo.hpp
    modules/instructions.hpp
    modules/collector.hpp
    modules/dlrm_defines.hpp
    hbm_traffic_top.hpp
    hbm_traffic_driver.hpp
    hbm_traffic_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design PUBLIC SystemC::systemc booksim noc dram)