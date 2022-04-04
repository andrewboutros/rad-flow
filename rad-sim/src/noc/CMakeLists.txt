cmake_minimum_required(VERSION 3.19)
find_package(SystemCLanguage CONFIG REQUIRED)

add_subdirectory(booksim)

include_directories(.
                    ../
                    /usr/local/include/
                    ${SystemCLanguage_DIR}/../include/
                    ./booksim
                    ./booksim/routers
                    ./booksim/allocators
                    ./booksim/arbiters
                    ./booksim/networks
                    ./booksim/power)

file(GLOB srcfiles "*.cpp")
file(GLOB hdrfiles "*.hpp")

add_library(noc STATIC ${srcfiles} ${hdrfiles})
target_link_libraries(noc SystemC::systemc booksim)