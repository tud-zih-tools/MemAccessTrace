#pragma once

#include <boost/circular_buffer.hpp>
#include <forward_list>
#include <vector>

#include <thread>

extern "C"
{
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <unistd.h>
}

enum class AccessType : uint32_t;
enum class MemoryLevel : uint32_t;
struct AccessEvent;
template <class Container> class EventBuffer;

using PointerSizePair = std::tuple<char*, uint64_t>;
using ConstPointerSizePair = std::tuple<const char*, uint64_t>;
using EventVectorBuffer = EventBuffer<std::vector<AccessEvent>>;
using EventRingBuffer = EventBuffer<boost::circular_buffer<AccessEvent>>;

inline std::string toString (AccessType access_type);
inline AccessType accessTypeFromString (const std::string& type);
inline AccessType accessTypeFromPerf (uint64_t mem_op);

inline std::string toString (MemoryLevel memory_level);
inline MemoryLevel memoryLevelFromPerf (uint64_t mem_lvl);

std::ostream& operator<< (std::ostream& os, const AccessEvent& access_event);

/*****************************************************************************
 * Access Types.
 *****************************************************************************/

enum class AccessType : uint32_t
{
    LOAD = PERF_MEM_OP_LOAD,
    STORE = PERF_MEM_OP_STORE,
    PREFETCH = PERF_MEM_OP_PFETCH,
    EXEC = PERF_MEM_OP_EXEC,
    NA = PERF_MEM_OP_NA,
};

inline std::string
toString (AccessType access_type)
{
    switch (access_type)
    {
    case AccessType::LOAD:
        return "Load";
    case AccessType::STORE:
        return "Store";
    case AccessType::PREFETCH:
        return "Prefetch";
    case AccessType::EXEC:
        return "Exec";
    case AccessType::NA:
        return "N/A";
    }
    return "Unsupported type";
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

/*****************************************************************************
 * Memory Levels
 *****************************************************************************/

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
toString (MemoryLevel memory_level)
{
    switch (memory_level)
    {
    case MemoryLevel::MEM_LVL_NA:
        return "N/A";
    case MemoryLevel::MEM_LVL_HIT:
        return "Hit";
    case MemoryLevel::MEM_LVL_MISS:
        return "Miss";
    case MemoryLevel::MEM_LVL_L1:
        return "L1";
    case MemoryLevel::MEM_LVL_L2:
        return "L2";
    case MemoryLevel::MEM_LVL_L3:
        return "L3";
    case MemoryLevel::MEM_LVL_LFB:
        return "Line Fill Buffer";
    case MemoryLevel::MEM_LVL_LOC_RAM:
        return "Local DRAM";
    case MemoryLevel::MEM_LVL_REM_RAM1:
        return "REM DRAM 1 hop";
    case MemoryLevel::MEM_LVL_REM_RAM2:
        return "REM DRAM 2 hops";
    case MemoryLevel::MEM_LVL_REM_CCE1:
        return "Remote cache 1 hop";
    case MemoryLevel::MEM_LVL_REM_CCE2:
        return "Remote cache 2 hops";
    case MemoryLevel::MEM_LVL_IO:
        return "I/O memory";
    case MemoryLevel::MEM_LVL_UNC:
        return "Uncached memory";
    }
    return "Unsupported memory level";
}

inline MemoryLevel
memoryLevelFromPerf (uint64_t mem_lvl)
{
    return static_cast<MemoryLevel> (mem_lvl);
}

/*****************************************************************************
 * Access Events
 *****************************************************************************/

struct AccessEvent
{
    AccessEvent ()
    {
    }
    AccessEvent (uint64_t t, uint64_t a, uint64_t i, AccessType at, MemoryLevel l)
    : time (t), address (a), ip (i), access_type (at), memory_level (l)
    {
    }
    uint64_t time = 0;
    uint64_t address = 0;
    uint64_t ip = 0;
    AccessType access_type = AccessType::NA;
    MemoryLevel memory_level = MemoryLevel::MEM_LVL_NA;
};

inline std::ostream&
operator<< (std::ostream& os, const AccessEvent& access_event)
{
    auto type = (access_event.access_type == AccessType::LOAD) ? "load" : "store";
    os << "Address " << std::hex << access_event.address << std::dec << ", Time " << access_event.time << ", IP "
       << std::hex << access_event.ip << std::dec << ", AccessType " << type;
    return os;
}

/*****************************************************************************
 * Event Buffer Interface
 *****************************************************************************/

template <class Container> class EventBuffer
{
    public:
    using const_iterator = typename Container::const_iterator;
    using iterator = typename Container::iterator;

    EventBuffer () = default;
    explicit EventBuffer (std::size_t size) : data_ (size)
    {
    }

    inline uint64_t
    size () const
    {
        return data_.size ();
    }

    inline uint64_t
    access_count () const
    {
        return access_count_;
    }

    inline void
    append (const AccessEvent& event)
    {
        access_count_++;
        data_.push_back (event);
    }

    std::forward_list<PointerSizePair>
    data ();

    std::forward_list<ConstPointerSizePair>
    data () const;

    inline const_iterator
    begin () const
    {
        return data_.begin();
    }

    inline const_iterator
    end () const
    {
        return data_.end();
    }

    inline iterator
    begin ()
    {
        return data_.begin();
    }

    inline iterator
    end ()
    {
        return data_.end();
    }

    inline const AccessEvent &
    operator[](size_t pos) const
    {
        return data_[pos];
    }

    inline AccessEvent &
    operator[](size_t pos)
    {
        return data_[pos];
    }

    private:
    Container data_;
    uint64_t access_count_ = 0;
};

/*****************************************************************************
 * Specialization for std::vector.
 *****************************************************************************/
template <>
inline std::forward_list<PointerSizePair>
EventVectorBuffer::data ()
{
    return { { reinterpret_cast<char*> (data_.data ()), data_.size () * sizeof (AccessEvent) } };
}

template <>
inline std::forward_list<ConstPointerSizePair>
EventVectorBuffer::data () const
{
    return { { reinterpret_cast<const char*> (data_.data ()), data_.size () * sizeof (AccessEvent) } };
}

template <>
inline uint64_t
EventVectorBuffer::size () const
{
    return data_.size();
}

/*****************************************************************************
 * Specialization for boost::circular_buffer.
 *****************************************************************************/

template <>
inline std::forward_list<PointerSizePair>
EventBuffer<boost::circular_buffer<AccessEvent>>::data ()
{
    std::forward_list<PointerSizePair> list;
    if (data_.array_two ().second > 0)
    {
        list.push_front (std::make_tuple (reinterpret_cast<char*> (data_.array_two ().first),
                                          data_.array_two ().second * sizeof (AccessEvent)));
    }
    list.push_front (std::make_tuple (reinterpret_cast<char*> (data_.array_one ().first),
                                      data_.array_one ().second * sizeof (AccessEvent)));
    return list;
}

template <>
inline std::forward_list<ConstPointerSizePair>
EventBuffer<boost::circular_buffer<AccessEvent>>::data () const
{
    std::forward_list<ConstPointerSizePair> list;
    if (data_.array_two ().second > 0)
    {
        list.push_front (std::make_tuple (reinterpret_cast<const char*> (data_.array_two ().first),
                                          data_.array_two ().second * sizeof (AccessEvent)));
    }
    list.push_front (std::make_tuple (reinterpret_cast<const char*> (data_.array_one ().first),
                                      data_.array_one ().second * sizeof (AccessEvent)));
    return list;
}

template <>
inline uint64_t
EventBuffer<boost::circular_buffer<AccessEvent>>::size() const
{
    return data_.size();
}