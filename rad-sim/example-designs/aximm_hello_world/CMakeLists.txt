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
  aximm_hello_world_top.cpp
  aximm_hello_world_driver.cpp
  aximm_hello_world_system.cpp
)

set(hdrfiles
  modules/responder.hpp
  modules/requester.hpp
  aximm_hello_world_top.hpp
  aximm_hello_world_driver.hpp
  aximm_hello_world_system.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(design STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(design PUBLIC SystemC::systemc booksim noc)