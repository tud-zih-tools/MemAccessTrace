#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <trace_events.h>
#include <trace_file.h>

PYBIND11_MODULE(tracefile, m)
{
    m.doc() = "Read/write trace files for sampled memory accesses";
}
