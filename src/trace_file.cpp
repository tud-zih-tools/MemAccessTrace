#include <cstring>
#include <iostream>

#include <trace_events.h>
#include <trace_file.h>

static auto
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

TraceFile::TraceFile (const FilePath& file, TraceFileMode mode)
{
    auto ios_mode = ios_open_mode (mode);
    file_.open (file, ios_mode | std::ios::binary);
}

TraceFile::~TraceFile ()
{
    file_.close ();
}

void
TraceFile::write (const EventBuffer& event_buffer, const TraceMetaData& md)
{
    write_meta_data (md);

    for (auto [pointer, size] : event_buffer.data ())
    {
        write_raw_data (pointer, size);
    }
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

AccessSequence
TraceFile::read ()
{
    TraceMetaData md;
    read_meta_data (&md);

    AccessSequence as (md.access_count());
    // TODO meta_data as parameter
    read_raw_data (reinterpret_cast<char*> (as.data ()), sizeof (AccessEvent) * md.access_count());
    return as;
}

void
TraceFile::read_meta_data (TraceMetaData* md)
{
    constexpr std::size_t tag_len = tag_.size ();
    char tag_buffer[tag_len];
    file_.read (tag_buffer, sizeof (char) * tag_len);
    if (tag_ != tag_buffer)
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
