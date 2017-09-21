cmake_minimum_required (VERSION 2.6)
project (measuro)
set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 1)
set(LIB_VERSION_REL 1)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/bin")
set(SRC_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/src")

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")
endif(CMAKE_COMPILER_IS_GNUCC)

install(FILES ${SRC_OUTPUT_PATH}/measuro.hpp DESTINATION .)

set(CMAKE_INSTALL_PREFIX "/usr/include/measuro")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_NAME "measuro-devel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Measuro: a fast, easy-to-use header library for creating program metrics in modern C++")
set(CPACK_PACKAGE_RELEASE "${LIB_VERSION_REL}")
set(CPACK_PACKAGE_VERSION "${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}")
set(CPACK_PACKAGE_CONTACT "hello@jamesmistry.com")
set(CPACK_PACKAGE_VENDOR "James Mistry")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}")
include(CPack)

enable_testing()

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/test")
