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
  modules/sender.cpp
  modules/receiver.cpp
  producer_consumer_system.cpp
  producer_consumer_top.cpp
)

set(hdrfiles
  modules/sender.hpp
  modules/receiver.hpp
  producer_consumer_system.hpp
  producer_consumer_top.hpp
)

add_compile_options(-Wall -Wextra -pedantic)

add_library(producer_consumer STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(producer_consumer PUBLIC SystemC::systemc booksim noc)