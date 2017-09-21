add_subdirectory(unit)
add_subdirectory(benchmark)
add_subdirectory(system)

add_test(measuro_static_analysis cppcheck --enable=warning,portability,information --std=c++11 --suppress=missingIncludeSystem ${SRC_OUTPUT_PATH}/measuro.hpp)
