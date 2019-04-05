add_executable(test_trace_file test_trace_file.cpp)
target_include_directories(test_trace_file PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(test_trace_file PRIVATE "${PROJECT_SOURCE_DIR}/lib/catch2")
target_link_libraries(test_trace_file PRIVATE boost_filesystem boost_system)
set_target_properties(test_trace_file PROPERTIES CXX_STANDARD 17)

install(TARGETS test_trace_file DESTINATION tests)

if(TRACEFILE_PYTHON_SUPPORT)
install(FILES test_trace_file.py DESTINATION tests)
endif(TRACEFILE_PYTHON_SUPPORT)