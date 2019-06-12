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

inline auto
ios_open_mode (TraceFileMode mode)
{
    switch (mode)
    {
    case TraceFileMode::READ:
        return std::ios::in;

    case TraceFileMode::WRITE:
        return std::ios::out;

    default:
        throw std::invalid_argument ("Unsupported open mode.");
    }
}

struct TraceMetaData
{
    public:
    TraceMetaData ()
    {
    }

    template <class T>
    explicit TraceMetaData (const EventBuffer<T>& event_buffer, const std::thread::id& tid)
    : size_ (event_buffer.size ()), tid_ (convert_thread_id (tid)), access_count_(event_buffer.access_count())
    {
    }

    template <class T>
    explicit TraceMetaData (const EventBuffer<T>& event_buffer, uint64_t tid)
    : size_ (event_buffer.size ()), tid_ (tid)
    {
    }

    uint64_t
    size () const
    {
        return size_;
    }

    uint64_t
    thread_id () const
    {
        return tid_;
    }

    uint64_t
    access_count() const
    {
        return access_count_;
    }

    private:
    uint64_t size_ = 0;
    uint64_t tid_ = 0;
    uint64_t access_count_ = 0;
};

class TraceFile
{

    public:
    explicit TraceFile (const FilePath& file, TraceFileMode mode)
    {
        auto ios_mode = ios_open_mode (mode);
        file_.open (file, ios_mode | std::ios::binary);
    }

    ~TraceFile ()
    {
        file_.close ();
    }

    template <class T>
    void
    write (const EventBuffer<T>& event_buffer, const TraceMetaData& md)
    {
        write_meta_data (md);

        for (auto [pointer, size] : event_buffer.data ())
        {
            write_raw_data (pointer, size);
        }
    }

    template <class T>
    std::tuple<EventBuffer<T>, TraceMetaData>
    read ()
    {
        TraceMetaData md;
        read_meta_data (&md);

        EventBuffer<T> buffer (md.size ());
        for (PointerSizePair data : buffer.data ())
        {
            read_raw_data (std::get<0> (data), std::get<1> (data));
        }

        return {buffer, md};
    }

    private:
    inline void
    write_meta_data (const TraceMetaData& md);

    inline void
    write_raw_data (const char* data, size_t nbytes);

    inline void
    read_meta_data (TraceMetaData* md);

    inline void
    read_raw_data (char* data, size_t nbytes);

    private:
    boost::filesystem::fstream file_;
    static constexpr std::string_view tag_ = "ATRACE";
};

template <>
inline std::tuple<EventBuffer<boost::circular_buffer<AccessEvent>>, TraceMetaData>
TraceFile::read ()
{
    TraceMetaData md;
    read_meta_data (&md);

    EventBuffer<boost::circular_buffer<AccessEvent>> buffer (md.size ());
    std::unique_ptr<AccessEvent[]> data = std::make_unique<AccessEvent[]> (md.size ());
    read_raw_data (reinterpret_cast<char*> (data.get ()), md.size () * sizeof(AccessEvent));

    for(uint64_t i = 0; i < md.size(); i++)
    {
        buffer.append(data.get () [i]);
    }

    return {buffer, md};
}

void
TraceFile::write_meta_data (const TraceMetaData& md)
{
    file_ << tag_;
    file_.write ((char*)&md, sizeof (TraceMetaData));
}

void
TraceFile::write_raw_data (const char* data, size_t nbytes)
{
    file_.write (data, nbytes);
}

void
TraceFile::read_meta_data (TraceMetaData* md)
{
    constexpr std::size_t tag_len = tag_.size ();
    char tag_buffer[tag_len + 1];
    file_.read (tag_buffer, sizeof (char) * tag_len);
    tag_buffer[tag_len] = '\0';
    if (tag_.compare(tag_buffer) != 0)
    {
        throw std::runtime_error ("Trace does not contain the correct tag at the beginning.");
    }
    file_.read ((char*)md, sizeof (TraceMetaData));
}

void
TraceFile::read_raw_data (char* data, size_t nbytes)
{
    file_.read (data, nbytes);
}
