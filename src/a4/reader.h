/**
 * ProtoBuf Reader
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef PROTOBUF_READER_H
#define PROTOBUF_READER_H

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "pb/Event.pb.h"
#include "pb/A4Stream.pb.h"

namespace fs = boost::filesystem;

class Reader
{
    public:
        Reader(const fs::path &input_file);
        ~Reader();

        bool good() const;
        uint32_t eventsRead() const;

        bool read_event(Event &);

    private:
        std::fstream _input;

        boost::shared_ptr< ::google::protobuf::io::ZeroCopyInputStream>
            _raw_in;

        boost::shared_ptr< ::google::protobuf::io::CodedInputStream>
            _coded_in;

        bool read_header();

        bool _is_good;
        uint32_t _events_stored;
        uint32_t _events_read;
        uint32_t _current_type;
};

#endif
