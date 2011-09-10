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

    /// Return structure for the InputStream

    /// If rr.error() is true the stream broke, if
    /// rr.end() the stream has terminated correctly.
    /// Contains a tuple (class_id, shared protobuf message)
    typedef struct ReadResult {
        /// Construct ReadResult that signifies end of stream or stream error
        ReadResult(bool error=false) { if (error) class_id = 1; else class_id = 0; object.reset(); };
        /// Construct normal ReadResult with class_id and protobuf Message
        ReadResult(uint32_t cls, shared<Message> obj) : class_id(cls), object(obj) {};
        /// Class ID of the message read
        uint32_t class_id;
        /// shared protobuf message 
        shared<Message> object;
        /// true if an error occurred
        bool error() const {return class_id == 1; };
        /// true if the stream has terminated correctly
        bool end() const {return class_id == 0; };
    } ReadResult;


    /// A4 Input Stream - reads protobuf messages from file

    /// A stream has "content objects" (aka events) and metadata.
    /// Get the next non-metadata object by calling next(),
    /// after that you can get the current_metadata()
    class A4InputStream
    {
        public:
            A4InputStream(shared<google::protobuf::io::ZeroCopyInputStream>, std::string name);
            /// Open the file @input_file for reading
            A4InputStream(const std::string & input_file);
            ~A4InputStream();

            /// Read the next message from the stream
            ReadResult next(bool internal=false);
            /// True if the stream can be read from
            bool is_good() {return _is_good;};
            /// True if new metadata has appeared since the last call to this function
            bool new_metadata() { if (_new_metadata) { _new_metadata = false; return true; } else return false;};
            uint64_t items_read() const {return _items_read;};
            /// Return a shared pointer to the current metadata message
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
