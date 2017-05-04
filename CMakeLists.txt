project(dancing-links)
cmake_minimum_required(VERSION 3.0)
include(ExternalProject)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -O2")

add_executable(sudoku dancing_links.hpp sudoku.cpp)

# GTest
set(GTEST_LOCATION ${CMAKE_BINARY_DIR}/gtest)
ExternalProject_Add(googletest
        GIT_REPOSITORY https://github.com/google/googletest
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GTEST_LOCATION}
        )

enable_testing()
include_directories(${GTEST_LOCATION}/include)
link_directories(${GTEST_LOCATION}/lib)

# Unit Tests
add_executable(tests_dancing_links dancing_links.hpp tests_lists_matrix.cpp tests_dancing_links.cpp)
add_dependencies(tests_dancing_links googletest)
target_link_libraries(tests_dancing_links gtest gtest_main pthread)
add_test( tests_dancing_links tests_dancing_links )



