#ifndef PROTOBUF_WRITER_H
#define PROTOBUF_WRITER_H

#include <string>

#include "a4/interfaces.h"

namespace google{ namespace protobuf{ namespace io{
    class FileOutputStream;
    class GzipOutputStream;
    class CodedOutputStream;
};};};

class Writer
{
    public:
        Writer(const std::string &output_file, const std::string description="", uint32_t content_class_id=0, uint32_t metadata_class_id=0, bool compression=true);
        ~Writer();

        bool write(Streamable& m);
        bool metadata(MetaData& m);

    private:
        ::google::protobuf::io::FileOutputStream * _raw_out;
        ::google::protobuf::io::GzipOutputStream * _compressed_out;
        ::google::protobuf::io::CodedOutputStream * _coded_out;

        bool write(uint32_t class_id, ::google::protobuf::Message& m);
        bool write_header(std::string description);
        bool write_footer();

        bool start_compression();
        bool stop_compression();

        uint64_t get_bytes_written();

        bool _compression;
        uint32_t _content_count;
        uint64_t _bytes_written;
        uint32_t _content_class_id;
        uint32_t _metadata_class_id;
};

#endif
