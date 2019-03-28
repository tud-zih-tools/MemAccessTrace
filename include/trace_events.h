#pragma once

#include <boost/circular_buffer.hpp>
#include <forward_list>
#include <thread>

extern "C"
{
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <unistd.h>
}

using PointerSizePair = std::tuple<const char*, uint64_t>;

enum class AccessType : uint32_t
{
    LOAD = PERF_MEM_OP_LOAD,
    STORE = PERF_MEM_OP_STORE,
    PREFETCH = PERF_MEM_OP_PFETCH,
    EXEC = PERF_MEM_OP_EXEC,
    NA = PERF_MEM_OP_NA,
};

inline std::string
toString(AccessType a)
{
    switch(a)
    {
        case AccessType::LOAD: return "Load";
        case AccessType::STORE: return "Store";
        case AccessType::PREFETCH: return "Prefetch";
        case AccessType::EXEC: return "Exec";
        case AccessType::NA: return "N/A";
    }
    return "Unsupported type";
}

enum class MemoryLevel : uint32_t
{
    MEM_LVL_NA = PERF_MEM_LVL_NA, //         Not available
    MEM_LVL_HIT = PERF_MEM_LVL_HIT, //        Hit
    MEM_LVL_MISS = PERF_MEM_LVL_MISS, //       Miss
    MEM_LVL_L1 = PERF_MEM_LVL_L1, //         Level 1 cache
    MEM_LVL_L2 = PERF_MEM_LVL_L2, //         Level 2 cache
    MEM_LVL_L3 = PERF_MEM_LVL_L3, //         Level 3 cache
    MEM_LVL_LFB = PERF_MEM_LVL_LFB, //        Line fill buffer
    MEM_LVL_LOC_RAM = PERF_MEM_LVL_LOC_RAM, //    Local DRAM
    MEM_LVL_REM_RAM1 = PERF_MEM_LVL_REM_RAM1, //   Remote DRAM 1 hop
    MEM_LVL_REM_RAM2 = PERF_MEM_LVL_REM_RAM2, //   Remote DRAM 2 hops
    MEM_LVL_REM_CCE1 = PERF_MEM_LVL_REM_CCE1, //   Remote cache 1 hop
    MEM_LVL_REM_CCE2 = PERF_MEM_LVL_REM_CCE2, //   Remote cache 2 hops
    MEM_LVL_IO = PERF_MEM_LVL_IO, //         I/O memory
    MEM_LVL_UNC = PERF_MEM_LVL_UNC, //        Uncached memory
};

inline std::string
toString(MemoryLevel ml)
{
    switch(ml)
    {
        case MemoryLevel::MEM_LVL_NA: return "N/A";
        case MemoryLevel::MEM_LVL_HIT: return "Hit";
        case MemoryLevel::MEM_LVL_MISS: return "Miss";
        case MemoryLevel::MEM_LVL_L1: return "L1";
        case MemoryLevel::MEM_LVL_L2: return "L2";
        case MemoryLevel::MEM_LVL_L3: return "L3";
        case MemoryLevel::MEM_LVL_LFB: return "Line Fill Buffer";
        case MemoryLevel::MEM_LVL_LOC_RAM: return "Local DRAM";
        case MemoryLevel::MEM_LVL_REM_RAM1: return "REM DRAM 1 hop";
        case MemoryLevel::MEM_LVL_REM_RAM2: return "REM DRAM 2 hops";
        case MemoryLevel::MEM_LVL_REM_CCE1: return "Remote cache 1 hop";
        case MemoryLevel::MEM_LVL_REM_CCE2: return "Remote cache 2 hops";
        case MemoryLevel::MEM_LVL_IO: return "I/O memory";
        case MemoryLevel::MEM_LVL_UNC: return "Uncached memory";
    }
    return "Unsupported memory level";
}

struct AccessEvent
{
    AccessEvent ()
    {
    }
    AccessEvent (uint64_t t, uint64_t a, uint64_t i, AccessType at, MemoryLevel l)
    : time (t), address (a), ip (i), access_type (at), memory_level (l)
    {
    }
    uint64_t time = 0; // 8 Byte
    uint64_t address = 0; // 8 Byte
    uint64_t ip = 0; // 8 Byte --> unw_word_t
    AccessType access_type = AccessType::NA; // 4 Byte
    MemoryLevel memory_level = MemoryLevel::MEM_LVL_NA; // 4 Byte
};
std::ostream&
operator<< (std::ostream& os, const AccessEvent& me);

struct EventBuffer
{

    std::thread::id tid;

    inline uint64_t
    access_count () const
    {
        return number_of_accesses_;
    }

    inline void
    add (uint64_t timestamp, uint64_t address, uint64_t instruction_pointer, AccessType type, MemoryLevel level)
    {
        number_of_accesses_++;
        data_.push_back (AccessEvent (timestamp, address, instruction_pointer, type, level));
    }

    inline std::forward_list<PointerSizePair>
    data () const
    {
        std::forward_list<PointerSizePair> list;
        if (data_.array_two ().second > 0)
        {
            list.push_front (std::make_tuple (reinterpret_cast<const char*> (data_.array_two ().first),
                                              data_.array_two ().second * sizeof (AccessEvent)));
        }
        list.push_front (std::make_tuple (reinterpret_cast<const char*> (data_.array_one ().first),
                                          data_.array_one ().second * sizeof (AccessEvent)));
        return list;
    }

    inline auto
    begin ()
    {
        return data_.begin ();
    }

    inline auto
    end ()
    {
        return data_.end ();
    }

    inline size_t
    size () const
    {
        return data_.size ();
    }

    explicit EventBuffer (std::size_t size) : data_ (size)
    {
    }

    private:
    boost::circular_buffer<AccessEvent> data_;
    uint64_t number_of_accesses_ = 0;
};

inline std::ostream&
operator<< (std::ostream& os, const AccessEvent& me)
{
    auto type = (me.access_type == AccessType::LOAD) ? "load" : "store";
    os << "Address " << std::hex << me.address << std::dec << ", Time " << me.time << ", IP "
       << std::hex << me.ip << std::dec << ", AccessType " << type;
    return os;
}

inline AccessType
accessTypeFromString (const std::string& type)
{
    if (type.find ("Load") != std::string::npos)
    {
        return AccessType::LOAD;
    }
    else if (type.find ("Store") != std::string::npos)
    {
        return AccessType::STORE;
    }
    else if (type.find ("Prefetch") != std::string::npos)
    {
        return AccessType::PREFETCH;
    }
    else if (type.find ("Exec") != std::string::npos)
    {
        return AccessType::EXEC;
    }

    return AccessType::NA;
}

inline AccessType
accessTypeFromPerf (uint64_t mem_op)
{
    return static_cast<AccessType> (mem_op);
}

inline MemoryLevel
memoryLevelFromPerf (uint64_t mem_lvl)
{
    return static_cast<MemoryLevel> (mem_lvl);
}
