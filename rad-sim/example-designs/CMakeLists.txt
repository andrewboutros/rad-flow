cmake_minimum_required(VERSION 3.16)
find_package(SystemCLanguage CONFIG REQUIRED)

#add_subdirectory(add)
#add_subdirectory(mult)

FOREACH(DESIGN_NAME ${DESIGN_NAMES})
    MESSAGE("<<${DESIGN_NAME}>>")
    add_subdirectory(${DESIGN_NAME})
ENDFOREACH()