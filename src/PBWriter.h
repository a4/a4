/**
 * ProtoBuf Writer
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef PROTOBUF_WRITER_H
#define PROTOBUF_WRITER_H

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "pb/Event.pb.h"

namespace fs = boost::filesystem;

namespace pb
{
    class Writer
    {
        public:
            Writer(const fs::path &output_file);
            ~Writer();

            bool write(const Event &);

        private:
            std::fstream _output;

            boost::shared_ptr< ::google::protobuf::io::ZeroCopyOutputStream>
                _raw_out;

            boost::shared_ptr< ::google::protobuf::io::ZeroCopyOutputStream>
                _raw_compressed_out;

            boost::shared_ptr< ::google::protobuf::io::CodedOutputStream>
                _coded_out;

            uint32_t _events_written;
    };
}

#endif
