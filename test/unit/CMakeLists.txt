cmake_minimum_required(VERSION 2.6)

include_directories(${SRC_OUTPUT_PATH})
link_directories("/usr/local/lib/gtest")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB test_src *.cpp)

add_executable(measuro_unit_testexe ${test_src})

target_link_libraries(measuro_unit_testexe gtest gtest_main pthread)

add_test(measuro_unit_testexe ${CMAKE_CURRENT_SOURCE_DIR}/measuro_unit_testexe)
