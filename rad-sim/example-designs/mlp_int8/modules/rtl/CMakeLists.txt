cmake_minimum_required(VERSION 3.16)

find_package(verilator CONFIG REQUIRED)
if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()
find_package(SystemCLanguage CONFIG REQUIRED)

set(verilator_options --pins-bv 2 -Wno-fatal -Wall)

set(rtl_top_modules
  rtl_mvm.v
)

set(rtl_libraries
  accum.v
  components.v
  datapath.v
  dpe.v
  reduce.v
)

add_library(rtl_designs STATIC)
target_link_libraries(rtl_designs PUBLIC SystemC::systemc)

foreach(module IN LISTS rtl_top_modules)
  foreach(rtl_library IN LISTS rtl_libraries)
    list(APPEND VERILOG_LIBRARIES "-v")
    list(APPEND VERILOG_LIBRARIES ${rtl_library})
  endforeach()

  verilate(rtl_designs
    SOURCES ${module}
    SYSTEMC
    VERILATOR_ARGS ${verilator_options} ${VERILOG_LIBRARIES}
  )
endforeach()