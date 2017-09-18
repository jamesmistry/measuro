add_subdirectory(unit)
add_subdirectory(benchmark)
add_subdirectory(system)

add_test(measuro_static_analysis cppcheck --check-config --enable=all --suppress=missingIncludeSystem ${PROJECT_SOURCE_DIR}/src/Measuro.hpp)
