#ifndef _A4_INPUT_STREAM_H_
#define _A4_INPUT_STREAM_H_

#include <string>
#include <vector>
#include <deque>

#include <a4/register.h>
#include <a4/message.h>

// used internally
namespace google{ namespace protobuf{ 
    namespace io{
        class ZeroCopyInputStream;
        class FileInputStream;
        class GzipInputStream;
        class CodedInputStream;
    };
    class SimpleDescriptorDatabase;
    class DescriptorPool;
    class DynamicMessageFactory;
};};

namespace a4{ namespace io{

    // used internally
    class GzipInputStream;
    class A4StartCompressedSection;
    class A4EndCompressedSection;
    class A4Proto;

    /// A4 Input Stream - reads protobuf messages from file

    /// A stream has "content message" (aka events) and metadata.
    /// Get the next non-metadata message by calling next(),
    /// after that you can get the current_metadata().
    class A4InputStream
    {
        public:
            A4InputStream(shared<google::protobuf::io::ZeroCopyInputStream>, std::string name);
            /// Open the file @input_file for reading
            A4InputStream(const std::string & input_file);
            ~A4InputStream();

            /// Read the next message from the stream.
            A4Message next(bool internal=false);
            /// Return the current metadata message.
            const A4Message current_metadata() {return _current_metadata; };
            /// True if new metadata has appeared since the last call to this function.
            bool new_metadata() { if (_new_metadata) { _new_metadata = false; return true; } else return false;};

            /// True if the stream has not ended or encountered an error.
            bool good() { return _good; };
            /// True if the stream has encountered an error.
            bool error() {return _error;};
            /// True if the stream has finished without error.
            bool end() {return !_error && !_good;};

            /// String representation of this stream for user output
            std::string str() { return std::string("A4InputStream(\"") + _inputname + "\")"; };
            
        private:
            shared<google::protobuf::io::ZeroCopyInputStream> _raw_in;
            shared<google::protobuf::io::FileInputStream> _file_in;
            shared<GzipInputStream> _compressed_in;
            shared<google::protobuf::io::CodedInputStream> _coded_in;

            // variables set at construction time
            std::string _inputname;
            uint32_t _content_class_id;
            uint32_t _metadata_class_id;
            internal::from_stream_func _content_func;
            
            shared<Message> message_factory(const google::protobuf::Message* prototype, google::protobuf::io::CodedInputStream *);
            void generate_dynamic_classes(const A4Proto* a4proto);
            shared<google::protobuf::SimpleDescriptorDatabase> _encountered_file_descriptors;
            shared<google::protobuf::DescriptorPool> _descriptor_pool;
            shared<google::protobuf::DynamicMessageFactory> _message_factory;

            // status variables
            bool _good, _error, _started,_discovery_complete;
            uint64_t _items_read;
            int _current_header_index;
            unsigned int _current_metadata_index;
            int _fileno;

            // metadata-related status
            bool _new_metadata, _current_metadata_refers_forward;
            A4Message _current_metadata;
            std::deque<std::vector<A4Message>> _metadata_per_header;
    
            // internal functions
            void init();
            void startup();
            void reset_coded_stream();
            bool discover_all_metadata();
            bool start_compression(const a4::io::A4StartCompressedSection& cs);
            bool stop_compression(const a4::io::A4EndCompressedSection& cs);
            bool read_header();
            int64_t seek(int64_t position, int whence);

            // set error/end status and return A4Message        
            A4Message set_error();
            A4Message set_end();
    };
};};

#endif
