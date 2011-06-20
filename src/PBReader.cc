/**
 * ProtoBuf Reader
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#include <math.h>

#include <string>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "PBReader.h"

using namespace std;

using pb::Reader;


const uint32_t HIGH_BIT = (1<<31);

Reader::Reader(const fs::path &input_file):
    _input(input_file.string().c_str(), ios::in | ios::binary),
    _is_good(true),
    _events_read(0)
{
    _raw_in.reset(new ::google::protobuf::io::IstreamInputStream(&_input));
    _coded_in.reset(new ::google::protobuf::io::CodedInputStream(_raw_in.get()));

    // Push limit of read bytes
    //
    _coded_in->SetTotalBytesLimit(pow(1024,3), 900*pow(1024,2));

    //
    _coded_in->ReadLittleEndian32(&_events_stored);
    //_coded_in->ReadBigEndian32(&_events_stored);
    read_header();


}

bool Reader::read_header()
{
    //uint64_t header_position = _coded_in->tell();

    string magic;
    if (!_coded_in->ReadString(&magic, 8))
        return false;
    if (! (magic == "A4STREAM"))
        return false;

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size))
        return false;

    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&_current_type))
            return false;
    }
    if (!_current_type == A4StreamHeader::kCLASSIDFieldNumber)
        return false;

    string message;
    if (!_coded_in->ReadString(&message, size))
        return false;
    // now ignore the message!
    return true;
}



bool Reader::read_event(Event &event)
{
    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size))
        return false;

    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&_current_type))
            return false;
    }

    string message;
    if (!_coded_in->ReadString(&message, size))
        return false;

    string magic;
    switch(_current_type) {
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
            if (! (magic == "KTHXBYE4"))
                return false;
            if (! read_header())
                return false;
            return read_event(event);
        case A4StreamHeader::kCLASSIDFieldNumber:
            // should not happen???
            return false;
            //return read_event(event);
    }
    return false; // unknown type
}

Reader::~Reader()
{
}

bool Reader::good() const
{
    return _is_good;
}

uint32_t Reader::eventsRead() const
{
    return _events_read;
}
