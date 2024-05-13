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
  modules/responder.cpp
  modules/requester.cpp
  accum_multiply_top.cpp
  accum_multiply_driver.cpp
  accum_multiply_system.cpp
)

set(hdrfiles
  modules/responder.hpp
  modules/requester.hpp
  accum_multiply_top.hpp
  accum_multiply_driver.hpp
  accum_multiply_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design PUBLIC SystemC::systemc booksim noc)