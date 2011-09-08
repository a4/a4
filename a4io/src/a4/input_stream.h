#ifndef _A4_INPUT_STREAM_H_
#define _A4_INPUT_STREAM_H_

#include <a4/stream.h>
#include <string>

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
            uint64_t items_read() const {return _items_read;};
            const shared<Message> last_meta_data() {return _last_meta_data; };

        private:
            shared<google::protobuf::io::ZeroCopyInputStream> _raw_in;
            shared<google::protobuf::io::FileInputStream> _file_in;
            GzipInputStream * _compressed_in;
            google::protobuf::io::CodedInputStream * _coded_in;

            void startup();
            int read_header();

            bool start_compression(const a4::io::A4StartCompressedSection& cs);
            bool stop_compression(const a4::io::A4EndCompressedSection& cs);
        
            std::string _inputname;
            bool _is_good;
            uint64_t _items_read;
            uint32_t _content_class_id;
            uint32_t _metadata_class_id;
            internal::from_stream_func _content_func;
            shared<Message> _last_meta_data;
    };
};};

#endif
