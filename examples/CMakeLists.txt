cmake_minimum_required(VERSION 2.6)

include_directories(${SRC_OUTPUT_PATH})

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB example_src *.cpp)

foreach(src_file ${example_src})
    get_filename_component(example_name ${src_file} NAME_WE)
    add_executable(${example_name}_exe ${src_file})
    target_link_libraries( ${example_name}_exe pthread )
endforeach(src_file ${example_src})

