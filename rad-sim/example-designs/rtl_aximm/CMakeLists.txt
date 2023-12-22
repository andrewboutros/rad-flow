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
  modules/client.cpp
  modules/server.cpp
  rtl_aximm_top.cpp
  rtl_aximm_driver.cpp
  rtl_aximm_system.cpp
)

set(hdrfiles
  modules/client.hpp
  modules/server.hpp
  rtl_aximm_top.hpp
  rtl_aximm_driver.hpp
  rtl_aximm_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design PUBLIC SystemC::systemc booksim noc rtl_designs)