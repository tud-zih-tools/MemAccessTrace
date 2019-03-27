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

class TraceFile
{

    public:
    explicit TraceFile (const FilePath& file, TraceFileMode mode);

    ~TraceFile ();

    void
    write (const EventBuffer& event_buffer);

    AccessSequence
    read ();

    private:
    struct TraceMetaData
    {
        uint64_t access_count;
        uint64_t tid;

        TraceMetaData () : access_count (0), tid (0)
        {
        }

        explicit TraceMetaData (const EventBuffer& event_buffer)
        : access_count (event_buffer.size ()), tid (convert_thread_id (event_buffer.tid))
        {
        }
    };

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