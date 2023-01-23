cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)

include_directories(
  ./
  ../
  ../../
  routers
  allocators
  arbiters
  networks
  power
  ${SystemCLanguage_DIR}/../src/
)

file(GLOB srcfiles
        "*.cpp"
        "allocators/*.cpp"
        "arbiters/*.cpp"
        "networks/*.cpp"
        "power/*.cpp"
        "routers/*.cpp"
        "y.tab.c"
        "lex.yy.c")

file(GLOB hdrfiles
        "*.hpp"
        "allocators/*.hpp"
        "arbiters/*.hpp"
        "networks/*.hpp"
        "power/*.hpp"
        "routers/*.hpp"
        "y.tab.h")

add_library(booksim STATIC ${srcfiles} ${hdrfiles})
