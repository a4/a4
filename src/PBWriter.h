/**
 * ProtoBuf Writer
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef PROTOBUF_WRITER_H
#define PROTOBUF_WRITER_H

#include <fstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "pb/A4Stream.pb.h"


namespace fs = boost::filesystem;
using std::string;

class Writer
{
    public:
        Writer(const fs::path &output_file, string content_name, uint32_t content_cls);
        ~Writer();

        typedef boost::shared_ptr< ::google::protobuf::Message> MessagePtr;
        bool write(MessagePtr m);

    private:


        std::fstream _output;

        boost::shared_ptr< ::google::protobuf::io::ZeroCopyOutputStream>
            _raw_out;

        boost::shared_ptr< ::google::protobuf::io::ZeroCopyOutputStream>
            _raw_compressed_out;

        boost::shared_ptr< ::google::protobuf::io::CodedOutputStream>
            _coded_out;

        bool write_header(string content_name);
        bool write_footer();

        uint32_t _content_count;
        uint32_t _bytes_written;
        uint32_t _content_type;
        uint32_t _previous_type;

};

#endif
