#ifndef _A4_INPUT_STREAM_IMPL_
#define _A4_INPUT_STREAM_IMPL_

#include <tuple>

#include "base_compressed_streams.h"
#include "proto_class_pool.h"
#include "zero_copy_resource.h"

#include <A4Stream.pb.h>

namespace a4{ namespace io{

    class InputStreamImpl
    {
        public:
            InputStreamImpl(unique<ZeroCopyStreamResource>, std::string name);
            virtual ~InputStreamImpl();

            /// Returns the next regular message in the stream.
            A4Message next(bool skip_metadata=true);
            /// Returns the next bare message in the stream.
            A4Message next_bare_message();
            /// Returns the next regular or metadata message in the stream.
            A4Message next_with_metadata() { return next(false);} ;
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
            
            size_t ByteCount() { return _raw_in->ByteCount(); }

            std::string str() { return _inputname; };
            
            const std::vector<std::vector<A4Message>>& all_metadata() {
                return _metadata_per_header;
            }

        private:
            unique<ZeroCopyStreamResource> _raw_in;
            shared<BaseCompressedInputStream> _compressed_in;
            shared<google::protobuf::io::CodedInputStream> _coded_in;
            shared<ProtoClassPool> _current_class_pool;

            // variables set at construction time
            std::string _inputname;

            // status variables
            bool _good, _error, _started, _discovery_complete;
            uint64_t _items_read;
            unsigned int _current_header_index;
            unsigned int _current_metadata_index;
            int _fileno;

            // metadata-related status
            bool _new_metadata, _current_metadata_refers_forward;
            A4Message _current_metadata;
            std::vector<std::vector<A4Message>> _metadata_per_header;
    
            // internal functions
            void startup();
            void reset_coded_stream();
            bool discover_all_metadata();
            bool start_compression(const a4::io::StartCompressedSection& cs);
            bool stop_compression(const a4::io::EndCompressedSection& cs);
            void drop_compression();
            bool read_header();
            int64_t seek(int64_t position);
            int64_t seek_back(int64_t position);
            A4Message bare_message();
            A4Message next_message();
            bool handle_compressed_section(A4Message & msg);
            bool handle_stream_command(A4Message & msg);
            bool handle_metadata(A4Message & msg);

            // set error/end status and return A4Message
            A4Message set_error();
            A4Message set_end();
    };

};};

#endif
