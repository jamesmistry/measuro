cmake_minimum_required(VERSION 2.6)

include_directories(${SRC_OUTPUT_PATH})

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

file(GLOB test_src *.cpp)

add_executable(measuro_benchmark_exe ${test_src})

