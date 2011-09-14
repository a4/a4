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

    class A4OutputStream
    {
        public:
            A4OutputStream(const std::string &output_file,
                           const std::string description="", 
                           uint32_t content_class_id=0, 
                           uint32_t metadata_class_id=0, 
                           bool metadata_refers_forward=false,
                           bool compression=true);
            A4OutputStream(shared<google::protobuf::io::ZeroCopyOutputStream>,
                           const std::string outname="<stream>",
                           const std::string description="", 
                           uint32_t content_class_id=0,
                           uint32_t metadata_class_id=0, 
                           bool metadata_refers_forward=false,
                           bool compression=true);
            ~A4OutputStream();

            void open();
            void close();
            bool opened() { return _opened; };
            bool closed() { return _closed; };

            bool write(google::protobuf::Message& m);
            bool metadata(google::protobuf::Message& m);

        private:
            shared<google::protobuf::io::ZeroCopyOutputStream> _raw_out;
            shared<google::protobuf::io::FileOutputStream> _file_out;
            google::protobuf::io::GzipOutputStream * _compressed_out;
            google::protobuf::io::CodedOutputStream * _coded_out;

            bool write(uint32_t class_id, google::protobuf::Message& m);
            bool write_header(std::string description);
            bool write_footer();


            bool start_compression();
            bool stop_compression();

            uint64_t get_bytes_written();
            void reset_coded_stream();

            std::string _output_name, _description;
            bool _compression;
            bool _opened, _closed;
            bool _metadata_refers_forward;
            uint32_t _content_count;
            uint64_t _bytes_written;
            uint32_t _content_class_id;
            uint32_t _metadata_class_id;
            std::vector<uint64_t> metadata_positions;

    };

};};

#endif
