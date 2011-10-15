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

            std::string str() { return _inputname; };

        private:
            unique<ZeroCopyStreamResource> _raw_in;
            shared<BaseCompressedInputStream> _compressed_in;
            shared<google::protobuf::io::CodedInputStream> _coded_in;
            std::vector<shared<ProtoClassPool>> _class_pools;

            // variables set at construction time
            std::string _inputname;

            // status variables
            bool _good, _error, _started, _discovery_complete;
            uint64_t _items_read;
            int _current_header_index;
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
            bool read_header();
            int64_t seek(int64_t position);
            int64_t seek_back(int64_t position);
            std::tuple<A4Message,uint32_t> next_message();
            A4Message next_message_msg() {return std::get<0>(next_message());};

            // set error/end status and return A4Message
            A4Message set_error();
            A4Message set_end();
    };

};};

#endif
