cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)
set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-11")
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-11")
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DRADSIM)
set(RADSIM 1)
add_subdirectory(noc)
add_subdirectory(design)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  /usr/local/include/
  ${SystemCLanguage_DIR}/../include/
  noc/booksim
  noc/booksim/routers
  noc/booksim/allocators
  noc/booksim/arbiters
  noc/booksim/networks
  noc/booksim/power
  noc
  design
  design/latency-insensitive-npu/inc/
)

file(GLOB srcfiles "*.cpp")
file(GLOB hdrfiles "*.hpp")
list(REMOVE_ITEM srcfiles "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
add_library(radsim STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(radsim PUBLIC
  SystemC::systemc
  booksim
  noc
  design
)

add_executable(system main.cpp ${srcfiles} ${hdrfiles})
add_custom_target(run_system
  COMMAND system
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Running system simulation ..."
)
add_custom_target(db_system
  COMMAND lldb system
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Running system simulation ..."
)
target_link_libraries(system PUBLIC
  SystemC::systemc
  booksim
  noc
  design
)