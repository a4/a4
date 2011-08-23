#include <math.h>

#include <string>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/reader.h"

using namespace std;

const uint32_t HIGH_BIT = (1<<31);

Reader::Reader(const fs::path &input_file):
    _input(input_file.string().c_str(), ios::in | ios::binary),
    _is_good(true),
    _events_read(0)
{
    _raw_in.reset(new ::google::protobuf::io::IstreamInputStream(&_input));
    _coded_in.reset(new ::google::protobuf::io::CodedInputStream(_raw_in.get()));

    // Push limit of read bytes
    _coded_in->SetTotalBytesLimit(pow(1024,3), 900*pow(1024,2));

    read_header();
}

bool Reader::read_header()
{
    //uint64_t header_position = _coded_in->tell();

    string magic;
    if (!_coded_in->ReadString(&magic, 8))
        return false;
    if (0 != magic.compare("A4STREAM"))
        return false;

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size))
        return false;

    uint32_t message_type = 0;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type))
            return false;
    }
    if (!message_type == A4StreamHeader::kCLASSIDFieldNumber)
        return false;

    string message;
    if (!_coded_in->ReadString(&message, size))
        return false;

    A4StreamHeader h;
    if (!h.ParseFromString(message))
        return false;

    // get the main content type from the header
    if (h.has_content_class_id())
        _content_type = h.content_class_id();
    else
        return false;

    return true;
}



bool Reader::read_event(Event &event)
{
    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size))
        return false;

    uint32_t message_type = _content_type;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type))
            return false;
    }

    string message;
    if (!_coded_in->ReadString(&message, size))
        return false;

    string magic;
    switch(message_type) {
        case Event::kCLASSIDFieldNumber:
            if (!event.ParseFromString(message))
                return false;
            ++_events_read;
            return true;
        case A4StreamFooter::kCLASSIDFieldNumber:
            // TODO: Process footer
            if (!_coded_in->ReadLittleEndian32(&size))
                return false;
            if (!_coded_in->ReadString(&magic, 8))
                return false;
            //cout << "read magic " << magic << endl;
            if (0 != magic.compare("KTHXBYE4"))
                return false;
            if (! read_header())
                return false;
            return read_event(event);
        case A4StreamHeader::kCLASSIDFieldNumber:
            // should not happen???
            return false;
            //return read_event(event);
    }
    return read_event(event); // unknown type, skip and read next event
}
