#ifndef _A4_OUTPUT_STREAM_H_
#define _A4_OUTPUT_STREAM_H_

#include <a4/stream.h>
#include <string>

namespace google{ namespace protobuf{ 
    class Message;
    namespace io{
        class ZeroCopyOutputStream;
        class FileOutputStream;
        class GzipOutputStream;
        class CodedOutputStream;
    };
};};

namespace a4{ namespace io{

    using google::protobuf::Message;

    class A4OutputStream
    {
        public:
            A4OutputStream(const std::string &output_file,
                           const std::string description="", 
                           uint32_t content_class_id=0, 
                           uint32_t metadata_class_id=0, 
                           bool compression=true);
            A4OutputStream(shared<google::protobuf::io::ZeroCopyOutputStream>,
                           const std::string outname="<stream>",
                           const std::string description="", 
                           uint32_t content_class_id=0,
                           uint32_t metadata_class_id=0, 
                           bool compression=true);
            ~A4OutputStream();

            bool write(Message& m);
            bool metadata(Message& m);

        private:
            shared<google::protobuf::io::ZeroCopyOutputStream> _raw_out;
            shared<google::protobuf::io::FileOutputStream> _file_out;
            google::protobuf::io::GzipOutputStream * _compressed_out;
            google::protobuf::io::CodedOutputStream * _coded_out;

            bool write(uint32_t class_id, google::protobuf::Message& m);
            bool write_header(std::string description);
            bool write_footer();

            void startup(std::string description);
            bool start_compression();
            bool stop_compression();

            uint64_t get_bytes_written();

            std::string _output_name;
            bool _compression;
            uint32_t _content_count;
            uint64_t _bytes_written;
            uint32_t _content_class_id;
            uint32_t _metadata_class_id;
    };

};};

#endif
