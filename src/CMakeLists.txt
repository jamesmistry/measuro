cmake_minimum_required (VERSION 2.6)

configure_file (
    "${CMAKE_SOURCE_DIR}/src/Measuro.hpp.in"
    "${SRC_OUTPUT_PATH}/Measuro.hpp"
)

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${SRC_OUTPUT_PATH}")


