cmake_minimum_required(VERSION 2.6)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${SRC_OUTPUT_PATH})

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

file(GLOB test_src *.cpp)

add_executable(measuro_unit_exe ${test_src})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # G++
  set(GCC_COVERAGE_COMPILE_FLAGS "--coverage")
  set_target_properties(measuro_unit_exe PROPERTIES COMPILE_FLAGS ${GCC_COVERAGE_COMPILE_FLAGS})
  target_link_libraries(measuro_unit_exe gtest gtest_main pthread gcov)
else()
  target_link_libraries(measuro_unit_exe gtest gtest_main pthread)
endif()

add_test(measuro_unit_test valgrind --error-exitcode=101 --tool=memcheck --leak-check=yes --show-leak-kinds=definite,possible --track-origins=yes ${EXECUTABLE_OUTPUT_PATH}/measuro_unit_exe)
