cmake_minimum_required(VERSION 3.16)
find_package(SystemCLanguage CONFIG REQUIRED)

add_subdirectory(booksim)

include_directories(
  ./
  ../
  ${SystemCLanguage_DIR}/../src/
  ./booksim
  ./booksim/routers
  ./booksim/allocators
  ./booksim/arbiters
  ./booksim/networks
  ./booksim/power
  ../../example-designs
  ../../inc
)

set(srcfiles
  axis_master_adapter.cpp
  axis_slave_adapter.cpp
  aximm_master_adapter.cpp
  aximm_slave_adapter.cpp
  radsim_noc.cpp
  noc_utils.cpp
  sc_flit.cpp
)

set(hdrfiles
  aximm_interface.hpp
  aximm_master_adapter.hpp
  aximm_slave_adapter.hpp
  axis_interface.hpp
  axis_master_adapter.hpp
  axis_slave_adapter.hpp
  radsim_noc.hpp
  noc_utils.hpp
  sc_flit.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(noc STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(noc SystemC::systemc booksim)
