cmake_minimum_required(VERSION 3.19)

find_package(verilator HINTS $ENV{VERILATOR_ROOT})
if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()
find_package(SystemCLanguage CONFIG REQUIRED)

set(rtlmodules
  adder.v
  client.v
)

add_library(rtl_designs STATIC)
target_link_libraries(rtl_designs PUBLIC SystemC::systemc)

foreach(module IN LISTS rtlmodules)
  verilate(rtl_designs
    SOURCES ${module}
    SYSTEMC
    VERILATOR_ARGS --pins-bv 2 -Wno-fatal -Wall
  )
endforeach()