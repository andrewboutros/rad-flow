cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)
set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-11")
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-11")
set(CMAKE_CXX_STANDARD 11)

include_directories(
  inc/
  /usr/local/include/
  ${SystemCLanguage_DIR}/../include/
)

if(DEFINED RADSIM)
  include_directories(../../)
endif()

file(GLOB srcfiles "src/*.cpp")
file(GLOB hdrfiles "inc/*.hpp")
list(REMOVE_ITEM srcfiles "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
add_library(npusim STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(npusim PUBLIC SystemC::systemc)

add_executable(npu main.cpp ${srcfiles} ${hdrfiles})
add_custom_target(run_standalone_npu
  COMMAND npu
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Running standalone NPU simulation ..."
)
target_link_libraries(npu PUBLIC SystemC::systemc)