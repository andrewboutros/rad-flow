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
  modules/mult.cpp
  modules/client_mult.cpp
  modules/portal_mult.cpp
  mult_top.cpp
  mult_driver.cpp
  mult_system.cpp
)

set(hdrfiles
  modules/mult.hpp
  modules/client_mult.hpp
  modules/portal_mult.hpp
  mult_top.hpp
  mult_driver.hpp
  mult_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design_mult STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design_mult PUBLIC SystemC::systemc booksim noc)