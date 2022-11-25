cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(noc)

include_directories(
  ./
  ${SystemCLanguage_DIR}/../include/
  noc
  noc/booksim
  noc/booksim/routers
  noc/booksim/allocators
  noc/booksim/arbiters
  noc/booksim/networks
  noc/booksim/power
  #../example-designs/npu
  #../example-designs/npu/modules
  ../example-designs/mvm
  ../example-designs/mvm/modules
)

set(srcfiles
  design_context.cpp
  radsim_config.cpp
  radsim_module.cpp
  radsim_telemetry.cpp
  radsim_utils.cpp
)

set(hdrfiles
  design_context.hpp
  radsim_config.hpp
  radsim_defines.hpp
  radsim_module.hpp
  radsim_telemetry.hpp
  radsim_utils.hpp
)

add_compile_options(-Wall -Wextra -pedantic)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(radsim STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(radsim PUBLIC SystemC::systemc booksim noc design)

add_executable(system main.cpp ${srcfiles} ${hdrfiles})
target_link_libraries(system PUBLIC radsim SystemC::systemc booksim noc design)
add_custom_target(run
  COMMAND system
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Running system simulation ..."
)

find_program(VALGRIND "valgrind")
if(VALGRIND)
    add_custom_target(valgrind
        COMMAND "${VALGRIND}" --tool=memcheck --leak-check=yes --undef-value-errors=no $<TARGET_FILE:system>)
endif()
