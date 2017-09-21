add_subdirectory(unit)
add_subdirectory(benchmark)
add_subdirectory(system)

add_test(measuro_static_analysis cppcheck --enable=all --std=c++11 --suppress=missingIncludeSystem ${PROJECT_SOURCE_DIR}/src/measuro.hpp)
