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
)

set(srcfiles
  modules/accum.cpp
  modules/asymmetric_fifo.cpp
  modules/axis_fifo_adapters.cpp
  modules/axis_inst_dispatch.cpp
  modules/axis_mvu_sector_chain.cpp
  modules/axis_mvu_sector.cpp
  modules/axis_vector_elementwise.cpp
  modules/decoders.cpp
  modules/dpe.cpp
  modules/evrf.cpp
  modules/fifo.cpp
  modules/instruction_fifo.cpp
  modules/instructions.cpp
  modules/loader.cpp
  modules/mfu.cpp
  modules/mvu_sector.cpp
  modules/piso_register_file.cpp
  modules/register_file.cpp
  modules/sim_utils.cpp
  npu_top.cpp
  npu_driver.cpp
  npu_system.cpp
)

set(hdrfiles
  modules/accum.hpp
  modules/asymmetric_fifo.hpp
  modules/axis_fifo_adapters.hpp
  modules/axis_inst_dispatch.hpp
  modules/axis_mvu_sector_chain.hpp
  modules/axis_mvu_sector.hpp
  modules/axis_vector_elementwise.hpp
  modules/decoders.hpp
  modules/dpe.hpp
  modules/evrf.hpp
  modules/fifo.hpp
  modules/instruction_fifo.hpp
  modules/instructions.hpp
  modules/interface_defines.hpp
  modules/loader.hpp
  modules/mfu.hpp
  modules/mvu_sector.hpp
  modules/params.hpp
  modules/piso_register_file.hpp
  modules/register_file.hpp
  modules/sim_utils.hpp
  npu_top.hpp
  npu_driver.hpp
  npu_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(npu STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(npu PUBLIC SystemC::systemc booksim noc)
