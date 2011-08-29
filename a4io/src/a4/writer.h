#ifndef PROTOBUF_WRITER_H
#define PROTOBUF_WRITER_H

#include <fstream>
#include <string>

#include <boost/shared_ptr.hpp>

#include <google/protobuf/message.h>

#include "a4/interfaces.h"

class Writer
{
    public:
        Writer(const std::string &output_file, const std::string description="", uint32_t content_class_id=0, uint32_t metadata_class_id=0);
        ~Writer();
        bool write(Streamable& m);

        bool metadata(MetaData& m);

    private:
        std::fstream _output;

        boost::shared_ptr< ::google::protobuf::io::ZeroCopyOutputStream>
            _raw_out;

        boost::shared_ptr< ::google::protobuf::io::ZeroCopyOutputStream>
            _raw_compressed_out;

        boost::shared_ptr< ::google::protobuf::io::CodedOutputStream>
            _coded_out;

        bool write(uint32_t class_id, ::google::protobuf::Message& m);
        bool write_header(std::string description);
        bool write_footer();

        uint32_t _content_count;
        uint32_t _bytes_written;
        uint32_t _content_class_id;
        uint32_t _metadata_class_id;
};

#endif
