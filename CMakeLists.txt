cmake_minimum_required(VERSION 3.15)

project(dancing-links LANGUAGES CXX) 

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "CMAKE_BUILD_TYPE is set to: ${CMAKE_BUILD_TYPE}")

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Sudoku example
add_executable(sudoku sudoku_main.cpp dancing_links.hpp sudoku.cpp)
target_compile_options(sudoku PRIVATE -Wall)

#
#  Tests
#

include(ExternalProject)
set(GTEST_INSTALL_DIR "${CMAKE_BINARY_DIR}/gtest_install") 
ExternalProject_Add(googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.17.0   
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_BUILD_TYPE=Release 
        BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libgtest.a
                         <INSTALL_DIR>/lib/libgtest_main.a
        INSTALL_DIR ${GTEST_INSTALL_DIR}
)

enable_testing()

add_executable(tests_dancing_links dancing_links.hpp tests_lists_matrix.cpp tests_dancing_links.cpp)
add_dependencies(tests_dancing_links googletest)

target_include_directories(tests_dancing_links PRIVATE ${GTEST_INSTALL_DIR}/include)
target_link_libraries(tests_dancing_links PRIVATE
    ${GTEST_INSTALL_DIR}/lib/libgtest.a
    ${GTEST_INSTALL_DIR}/lib/libgtest_main.a
    pthread
)
target_compile_options(tests_dancing_links PRIVATE -Wall)

add_test(NAME tests_dancing_links COMMAND tests_dancing_links)

#
# Benchmarks
#

# benchmark
ExternalProject_Add(
    google_benchmark_external
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4
    SOURCE_DIR "${CMAKE_BINARY_DIR}/extern/benchmark_src"
    BINARY_DIR "${CMAKE_BINARY_DIR}/extern/benchmark_build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/extern/benchmark_install"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DBENCHMARK_ENABLE_GTEST_TESTS=OFF
        -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON # Ensures necessary dependencies are downloaded
        -DCMAKE_BUILD_TYPE=Release 
)

add_executable(benchmark_dancing_links benchmark_dancing_links.cpp sudoku.cpp)

target_include_directories(benchmark_dancing_links PRIVATE
    "${CMAKE_BINARY_DIR}/extern/benchmark_install/include"
)

target_link_libraries(benchmark_dancing_links PRIVATE
    "${CMAKE_BINARY_DIR}/extern/benchmark_install/lib/libbenchmark.a"
    "${CMAKE_BINARY_DIR}/extern/benchmark_install/lib/libbenchmark_main.a"
)

add_dependencies(benchmark_dancing_links google_benchmark_external)
