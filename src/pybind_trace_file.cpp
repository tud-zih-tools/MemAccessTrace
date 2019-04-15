#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <trace_events.h>
#include <trace_file.h>

namespace py = pybind11;

template<class Container>
void declare_event_buffer(py::module &m, const char * pyclass_name)
{
    py::class_<Container> (m, pyclass_name)
    .def (py::init<> ())
    .def (py::init<std::size_t> ())
    .def ("append", py::overload_cast<const AccessEvent&> (&Container::append))
    .def ("__len__", &Container::size)
    .def ("__iter__", [](Container& eb)
                      { return py::make_iterator (eb.begin (), eb.end ()); },
                      py::keep_alive<0, 1> ())
    .def ("__str__", [](const Container & buffer)
                     {
                        std::stringstream ss;
                        ss << "[";
                        for(const AccessEvent & e : buffer)
                        {
                            ss << e << ",";
                        }
                        ss << "]";
                        return ss.str();
                     })
    .def ("__repr__", [](const Container & buffer)
                      {
                        py::object obj = py::cast(&buffer);
                        return py::str(obj);
                      })
    .def("__getitem__", [](const Container & buffer, ssize_t index)
                      {
                        return buffer[index];
                      })
    .def("__setitem__", [](Container & buffer, ssize_t index, AccessEvent & access)
                      {
                        buffer[index] = access;
                      });
}

PYBIND11_MODULE (tracefile, m)
{
    m.doc () =
    "The module provides an API to write recorded memory accesses to a trace file."
    "tracefiles provides also classes for storing access events and their relevant information.";

    py::enum_<TraceFileMode> (m, "TraceFileMode")
    .value ("READ", TraceFileMode::READ)
    .value ("WRITE", TraceFileMode::WRITE);

    py::enum_<AccessType> (m, "AccessType")
    .value ("LOAD", AccessType::LOAD)
    .value ("STORE", AccessType::STORE)
    .value ("PREFETCH", AccessType::PREFETCH)
    .value ("EXEC", AccessType::EXEC)
    .value ("NA", AccessType::NA);

    py::enum_<MemoryLevel> (m, "MemoryLevel")
    .value ("MEM_LVL_NA", MemoryLevel::MEM_LVL_NA)
    .value ("MEM_LVL_HIT", MemoryLevel::MEM_LVL_HIT)
    .value ("MEM_LVL_MISS", MemoryLevel::MEM_LVL_MISS)
    .value ("MEM_LVL_L1", MemoryLevel::MEM_LVL_L1)
    .value ("MEM_LVL_L2", MemoryLevel::MEM_LVL_L2)
    .value ("MEM_LVL_L3", MemoryLevel::MEM_LVL_L3)
    .value ("MEM_LVL_LFB", MemoryLevel::MEM_LVL_LFB)
    .value ("MEM_LVL_LOC_RAM", MemoryLevel::MEM_LVL_LOC_RAM)
    .value ("MEM_LVL_REM_RAM1", MemoryLevel::MEM_LVL_REM_RAM1)
    .value ("MEM_LVL_REM_RAM2", MemoryLevel::MEM_LVL_REM_RAM2)
    .value ("MEM_LVL_REM_CCE1", MemoryLevel::MEM_LVL_REM_CCE1)
    .value ("MEM_LVL_REM_CCE2", MemoryLevel::MEM_LVL_REM_CCE2)
    .value ("MEM_LVL_IO", MemoryLevel::MEM_LVL_IO)
    .value ("MEM_LVL_UNC", MemoryLevel::MEM_LVL_UNC);

    py::class_<AccessEvent> (m, "AccessEvent")
    .def (py::init<uint64_t, uint64_t, uint64_t, AccessType, MemoryLevel> (),
          py::arg ("timestamp") = 0, py::arg ("address") = 0, py::arg ("ip") = 0,
          py::arg ("type") = AccessType::NA, py::arg ("level") = MemoryLevel::MEM_LVL_NA)
    .def_readonly ("timestamp", &AccessEvent::time)
    .def_readonly ("address", &AccessEvent::address)
    .def_readonly ("ip", &AccessEvent::ip)
    .def_readonly ("type", &AccessEvent::access_type)
    .def_readonly ("level", &AccessEvent::memory_level)
    .def ("__str__", [](const AccessEvent& a)
                     {
                        std::stringstream ss;
                        ss << "[Timestamp: " << a.time << ", "
                        << "Address: " << std::hex << a.address << ", "
                        << "IP: " << std::hex << a.ip << ", "
                        << "Type: " << toString (a.access_type) << ", "
                        << "Level: " << toString (a.memory_level) << "]";
                        return ss.str ();
                     })
    .def ("__repr_", [](const AccessEvent& a)
                     {
                         py::object obj = py::cast(&a);
                         return py::str(obj);
                     });

    declare_event_buffer<EventVectorBuffer>(m, "EventVectorBuffer");
    declare_event_buffer<EventRingBuffer>(m, "EventRingBuffer");

    py::class_<TraceMetaData>(m, "TraceMetaData")
    .def(py::init<const EventRingBuffer&, uint64_t>())
    .def(py::init<const EventVectorBuffer&, uint64_t>())
    .def("size", &TraceMetaData::size)
    .def("thread_id", &TraceMetaData::thread_id)
    .def("__str__", [](const TraceMetaData & md)
                    {
                        std::stringstream ss;
                        ss << "[ThreadId: " << md.thread_id() << ",";
                        ss << " Size: " << md.size() << "]";
                        return ss.str();
                    })
    .def("__repr__", [](const TraceMetaData & md)
                     {
                         py::object obj = py::cast(&md);
                         return py::str(obj);
                     });
}
