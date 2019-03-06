project(trace_file)

cmake_minimum_required(VERSION 3.8)

add_subdirectory(test)

add_library(trace_file
            SHARED
            src/trace_file.cpp)

set_target_properties(trace_file PROPERTIES CXX_STANDARD 17)

target_include_directories(trace_file PUBLIC include)

target_compile_options(trace_file INTERFACE -Wall -Wextra)
