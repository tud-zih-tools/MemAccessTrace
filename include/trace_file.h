#pragma once
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <trace_events.h>

using FilePath = boost::filesystem::path;
using AccessSequence = std::vector<AccessEvent>;

enum class TraceFileMode
{
    READ,
    WRITE,
};

inline size_t
convert_thread_id (std::thread::id tid)
{
    std::stringstream ss;
    ss << tid;
    return std::stoull (ss.str ());
}

struct TraceMetaData
{
    public:
    TraceMetaData ()
    {
    }
    explicit TraceMetaData (const EventBuffer& event_buffer, const std::thread::id& tid)
    : access_count_ (event_buffer.size ()), tid_ (convert_thread_id (tid))
    {
    }

    uint64_t
    access_count () const
    {
        return access_count_;
    }

    uint64_t
    thread_id () const
    {
        return tid_;
    }

    private:
    uint64_t access_count_ = 0;
    uint64_t tid_ = 0;
};

class TraceFile
{

    public:
    explicit TraceFile (const FilePath& file, TraceFileMode mode);

    ~TraceFile ();

    void
    write (const EventBuffer& event_buffer, const TraceMetaData& meta_data);

    AccessSequence
    read ();

    private:
    void
    write_meta_data (const TraceMetaData& md);

    void
    write_raw_data (const char* data, size_t nbytes);

    void
    read_meta_data (TraceMetaData* md);

    void
    read_raw_data (char* data, size_t nbytes);

    private:
    boost::filesystem::fstream file_;
    static constexpr std::string_view tag_ = "ATRACE";
};