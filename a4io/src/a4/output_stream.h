#ifndef _A4_OUTPUT_STREAM_H_
#define _A4_OUTPUT_STREAM_H_

#include <string>
#include <vector>
#include <cassert>

#include <google/protobuf/descriptor.h>

#include <a4/register.h>


namespace google{ namespace protobuf{ 
    namespace io{
        class ZeroCopyOutputStream;
        class FileOutputStream;
        class GzipOutputStream;
        class CodedOutputStream;
    };
};};

namespace a4{ namespace io{

    /// Class to write Messages to files or streams.
    /// To write a message belonging to a certain class
    /// make sure you told @UseClassID about it,
    /// by writing "template UseClassId<MyProtobufClass>;"
    /// somewhere in a cpp file.
    class A4OutputStream
    {
        public:
            A4OutputStream(const std::string &output_file,
                           const std::string description="", 
                           uint32_t content_class_id=0, 
                           uint32_t metadata_class_id=0);
            A4OutputStream(shared<google::protobuf::io::ZeroCopyOutputStream>,
                           const std::string outname="<stream>",
                           const std::string description="", 
                           uint32_t content_class_id=0,
                           uint32_t metadata_class_id=0);
            ~A4OutputStream();

            /// Write a message to the stream
            /// Take care to respect the metadata message direction - forward means the events following
            /// the metadata are described, otherwise the previous events are referred to.
            bool write(const google::protobuf::Message& m);

            /// \internal Explicitly close the file. \endinternal
            bool close();
            bool opened() { return _opened; };
            bool closed() { return _closed; };
            
            /// Set compression flag [default=true]
            A4OutputStream & set_compression(bool c) { _compression = c; return *this; };
            /// If called, metadata will refer to the events following the metadata, instead of events before.
            /// Has to be called before writing is begun.
            A4OutputStream & set_forward_metadata() { assert(!_opened); _metadata_refers_forward = true; return *this; };
            /// Set the content message object ( s->content_cls<TestEvent>(); ).
            /// Has to be called before writing is begun.
            A4OutputStream & content_cls(uint32_t id) { assert(!_opened); _content_class_id = id; return *this; };
            template<class ProtoClass>
            A4OutputStream & content_cls() {
                assert(!_opened);
                _content_class_id = ProtoClass::kCLASSIDFieldNumber;
                return *this;
            };
            /// Set the metadata message object ( s->metadata_cls<TestEvent>(); ).
            /// Has to be called before writing is begun.
            A4OutputStream & metadata_cls(uint32_t id) { assert(!_opened); _metadata_class_id = id; return *this; };
            template<class ProtoClass>
            A4OutputStream & metadata_cls() {
                assert(!_opened);
                _metadata_class_id = ProtoClass::kCLASSIDFieldNumber;
                return *this;
            };

            /// String representation of this stream for user output
            std::string str() { return std::string("A4OutputStream(\"") + _output_name + "\", \"" + _description + "\")"; };
        private:
            shared<google::protobuf::io::ZeroCopyOutputStream> _raw_out;
            shared<google::protobuf::io::FileOutputStream> _file_out;
            google::protobuf::io::GzipOutputStream * _compressed_out;
            google::protobuf::io::CodedOutputStream * _coded_out;

            bool open();
            bool write(uint32_t class_id, const google::protobuf::Message& m);
            bool write_header(std::string description);
            bool write_footer();
            bool start_compression();
            bool stop_compression();

            uint64_t get_bytes_written();
            void reset_coded_stream();

            std::string _output_name, _description;
            int _fileno;
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
