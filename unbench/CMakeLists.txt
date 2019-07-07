# 3.11 version is required for `FetchContent`
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(ecs.hpp.unbench)

#
# executable
#

file(GLOB UNBENCH_SOURCES "*.cpp" "*.hpp")
add_executable(${PROJECT_NAME} ${UNBENCH_SOURCES})
target_link_libraries(${PROJECT_NAME} ecs.hpp)

target_compile_options(${PROJECT_NAME}
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall -Wextra -Wpedantic>)

add_test(${PROJECT_NAME} ${PROJECT_NAME})

#
# google/benchmark
#

include(FetchContent)
FetchContent_Declare(
    google_benchmark
    GIT_REPOSITORY https://github.com/google/benchmark)

FetchContent_GetProperties(google_benchmark)
if(NOT google_benchmark_POPULATED)
    FetchContent_Populate(google_benchmark)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    add_subdirectory(${google_benchmark_SOURCE_DIR} ${google_benchmark_BINARY_DIR})
    target_link_libraries(${PROJECT_NAME} benchmark_main)
endif()
