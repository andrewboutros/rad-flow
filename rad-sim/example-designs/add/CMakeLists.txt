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
  modules/adder.cpp
  modules/client.cpp
  modules/fifo.cpp
  add_top.cpp
  add_driver.cpp
  add_system.cpp
)

set(hdrfiles
  modules/adder.hpp
  modules/client.hpp
  modules/fifo.hpp
  add_top.hpp
  add_driver.hpp
  add_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design PUBLIC SystemC::systemc booksim noc)