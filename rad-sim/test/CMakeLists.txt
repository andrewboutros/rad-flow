cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)

include_directories(
  ./
  ${SystemCLanguage_DIR}/../include/
  ../src
  ../src/noc
  ../src/noc/booksim
  ../src/noc/booksim/routers
  ../src/noc/booksim/allocators
  ../src/noc/booksim/arbiters
  ../src/noc/booksim/networks
  ../src/noc/booksim/power
  /usr/local/include/
)

add_executable(test_axis_adapters axis_adapters_noc_test.cpp axis_adapters_noc_test.hpp)

add_custom_target(run_tests
  COMMAND test_axis_adapters 2> cerr.log
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Running test executables ..."
)

target_link_libraries(test_axis_adapters PUBLIC 
  radsim 
  SystemC::systemc 
  booksim
  noc
)