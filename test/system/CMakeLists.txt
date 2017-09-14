cmake_minimum_required(VERSION 2.6)

link_directories("/usr/local/lib/gtest")
include_directories(${SRC_OUTPUT_PATH})

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB test_src *.cpp)

add_executable(measuro_system_testexe ${test_src})

target_link_libraries(measuro_system_testexe gtest pthread)

add_test(measuro_system_test python ${CMAKE_CURRENT_SOURCE_DIR}/check_output.py ${CMAKE_CURRENT_SOURCE_DIR}/measuro_system_testexe)

