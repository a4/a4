#ifndef _A4_INPUT_STREAM_H_
#define _A4_INPUT_STREAM_H_

#include <a4/stream.h>
#include <string>
#include <vector>
#include <deque>

namespace google{ namespace protobuf{
    class Message;
    namespace io{
        class ZeroCopyInputStream;
        class FileInputStream;
        class GzipInputStream;
        class CodedInputStream;
    };
};};

namespace a4{ namespace io{

    using google::protobuf::Message;

    class GzipInputStream;
    class A4StartCompressedSection;
    class A4EndCompressedSection;

    typedef struct RR {
        RR(uint32_t cls, shared<Message> obj) : class_id(cls), object(obj) {};
        uint32_t class_id;
        shared<Message> object;
        bool operator==(const RR & rhs) {return rhs.class_id==class_id && rhs.object==object;}
    } ReadResult;
    const ReadResult END_OF_STREAM = {0, shared<Message>()};
    const ReadResult READ_ERROR = {1, shared<Message>()};

    class A4InputStream
    {
        public:
            A4InputStream(shared<google::protobuf::io::ZeroCopyInputStream>, std::string name);
            A4InputStream(const std::string & input_file);
            ~A4InputStream();

            ReadResult next();
            bool is_good() {return _is_good;};
            bool new_metadata() { if (_new_metadata) { _new_metadata = false; return true; } else return false;};
            uint64_t items_read() const {return _items_read;};
            const shared<Message> current_metadata() {return _current_metadata; };

        private:
            int _fileno;
            shared<google::protobuf::io::ZeroCopyInputStream> _raw_in;
            shared<google::protobuf::io::FileInputStream> _file_in;
            GzipInputStream * _compressed_in;
            google::protobuf::io::CodedInputStream * _coded_in;

            void startup();
            int read_header();
            void reset_coded_stream();
            int64_t seek(int64_t position, int whence);

            bool start_compression(const a4::io::A4StartCompressedSection& cs);
            bool stop_compression(const a4::io::A4EndCompressedSection& cs);
            bool discover_all_metadata();
        
            std::string _inputname;
            bool _is_good, _new_metadata, _discovery_complete;
            uint64_t _items_read;
            uint32_t _content_class_id;
            uint32_t _metadata_class_id;
            internal::from_stream_func _content_func;
            shared<Message> _current_metadata;

            bool _current_metadata_refers_forward;

            int _current_header_index;
            int _current_metadata_index;
            std::deque<std::vector<shared<Message>>> _metadata_per_header;

    };
};};

#endif
