#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <trace_events.h>
#include <trace_file.h>

namespace py = pybind11;

PYBIND11_MODULE(tracefile, m)
{
    m.doc() = "The module provides an API to write recorded memory accesses to a trace file."
              "tracefiles provides also classes for storing access events and their relevant information.";

    py::enum_<AccessType>(m, "AccessType")
        .value("LOAD", AccessType::LOAD)
        .value("STORE", AccessType::STORE)
        .value("PREFETCH", AccessType::PREFETCH)
        .value("EXEC", AccessType::EXEC)
        .value("NA", AccessType::NA);

    py::enum_<MemoryLevel>(m, "MemoryLevel")
        .value("MEM_LVL_NA", MemoryLevel::MEM_LVL_NA)
        .value("MEM_LVL_HIT", MemoryLevel::MEM_LVL_HIT)
        .value("MEM_LVL_MISS", MemoryLevel::MEM_LVL_MISS)
        .value("MEM_LVL_L1", MemoryLevel::MEM_LVL_L1)
        .value("MEM_LVL_L2", MemoryLevel::MEM_LVL_L2)
        .value("MEM_LVL_L3", MemoryLevel::MEM_LVL_L3)
        .value("MEM_LVL_LFB", MemoryLevel::MEM_LVL_LFB)
        .value("MEM_LVL_LOC_RAM", MemoryLevel::MEM_LVL_LOC_RAM)
        .value("MEM_LVL_REM_RAM1", MemoryLevel::MEM_LVL_REM_RAM1)
        .value("MEM_LVL_REM_RAM2", MemoryLevel::MEM_LVL_REM_RAM2)
        .value("MEM_LVL_REM_CCE1", MemoryLevel::MEM_LVL_REM_CCE1)
        .value("MEM_LVL_REM_CCE2", MemoryLevel::MEM_LVL_REM_CCE2)
        .value("MEM_LVL_IO", MemoryLevel::MEM_LVL_IO)
        .value("MEM_LVL_UNC", MemoryLevel::MEM_LVL_UNC);

    py::class_<AccessEvent>(m, "AccessEvent")
        .def(py::init<uint64_t, uint64_t, uint64_t, AccessType, MemoryLevel>(),
             py::arg("timestamp") = 0, py::arg("address") = 0, py::arg("ip") = 0,
             py::arg("type") = AccessType::NA, py::arg("level") = MemoryLevel::MEM_LVL_NA)
        .def_readonly("timestamp", &AccessEvent::time)
        .def_readonly("address", &AccessEvent::address)
        .def_readonly("ip", &AccessEvent::ip)
        .def_readonly("type", &AccessEvent::access_type)
        .def_readonly("level", &AccessEvent::memory_level)
        .def("__str__",
             [](const AccessEvent& a)
             {
                 std::stringstream ss;
                 ss << "[Timestamp: " << a.time << ", "
                    << "Address: " << std::hex << a.address << ", "
                    << "IP: " << std::hex << a.ip << ", "
                    << "Type: " << toString(a.access_type) << ", "
                    << "Level: " << toString(a.memory_level) << "]";
                return ss.str();
             });
}
