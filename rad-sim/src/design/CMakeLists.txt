cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)

add_subdirectory(latency-insensitive-npu)

include_directories(
  ./
  ../
  ./latency-insensitive-npu/inc/
  ${SystemCLanguage_DIR}/../include/
  ../noc
  ../noc/booksim
  ../noc/booksim/routers
  ../noc/booksim/allocators
  ../noc/booksim/arbiters
  ../noc/booksim/networks
  ../noc/booksim/power
  /usr/local/include/
)

file(GLOB srcfiles "*.cpp")
file(GLOB hdrfiles "*.hpp")
add_library(design STATIC ${srcfiles} ${hdrfiles})

target_link_libraries(design PUBLIC
  SystemC::systemc
  booksim
  noc
  npusim
)