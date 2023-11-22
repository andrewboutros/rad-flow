cmake_minimum_required(VERSION 3.16)
find_package(SystemCLanguage CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(noc)
add_subdirectory(dram)

include_directories(
  ./
  ${SystemCLanguage_DIR}/../src/
  noc
  noc/booksim
  noc/booksim/routers
  noc/booksim/allocators
  noc/booksim/arbiters
  noc/booksim/networks
  noc/booksim/power
  dram
  dram/DRAMsim3
  dram/DRAMsim3/src
  ../example-designs/${DESIGN}
  ../example-designs/${DESIGN}/modules
)

find_package(verilator CONFIG)
if (verilator_FOUND)
    include_directories(SYSTEM "${VERILATOR_ROOT}/include")
endif()

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
target_link_libraries(radsim PUBLIC SystemC::systemc booksim noc dram design)

add_executable(system main.cpp ${srcfiles} ${hdrfiles})
target_link_libraries(system PUBLIC radsim SystemC::systemc booksim noc dram design)
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
