project(trace_file)

cmake_minimum_required(VERSION 3.8)

set (CMAKE_CXX_STANDARD 17)

find_package(PythonInterp)
find_package(PythonLibs)

set(TRACEFILE_PYTHON_SUPPORT PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)

add_subdirectory(test)

add_library(trace_file
            SHARED
            src/trace_file.cpp)

target_include_directories(trace_file PUBLIC include)

target_compile_options(trace_file INTERFACE -Wall -Wextra)

if(TRACEFILE_PYTHON_SUPPORT)
    add_subdirectory(pybind11)

    add_library(tracefile MODULE src/pybind_trace_file.cpp)

    target_include_directories(tracefile PRIVATE include)
    target_link_libraries(tracefile PRIVATE pybind11::module)
    set_target_properties(tracefile PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
                                               SUFFIX "${PYTHON_MODULE_EXTENSION}")

    install(TARGETS tracefile DESTINATION "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages")
endif(TRACEFILE_PYTHON_SUPPORT)

install(TARGETS trace_file DESTINATION lib)

install(FILES include/trace_events.h include/trace_file.h DESTINATION include)