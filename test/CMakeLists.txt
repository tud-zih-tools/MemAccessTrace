add_executable(test_trace_file test_trace_file.cpp
               "${PROJECT_SOURCE_DIR}/src/trace_file.cpp")
target_include_directories(test_trace_file PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(test_trace_file PRIVATE "${PROJECT_SOURCE_DIR}/lib/catch2")
target_link_libraries(test_trace_file PRIVATE boost_filesystem boost_system)
set_target_properties(test_trace_file PROPERTIES CXX_STANDARD 17)