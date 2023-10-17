cmake_minimum_required(VERSION 3.19)
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

set(srcfiles
  modules/adder_wrapper.cpp
  modules/client_wrapper.cpp
  rtl_add_top.cpp
  rtl_add_driver.cpp
  rtl_add_system.cpp
)

set(hdrfiles
  modules/adder_wrapper.hpp
  modules/client_wrapper.hpp
  rtl_add_top.hpp
  rtl_add_driver.hpp
  rtl_add_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design PUBLIC SystemC::systemc booksim noc rtl_designs)