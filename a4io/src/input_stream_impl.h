#ifndef _A4_INPUT_STREAM_IMPL_
#define _A4_INPUT_STREAM_IMPL_

#include <tuple>

#include <a4/types.h>

#include "base_compressed_streams.h"
#include "proto_class_pool.h"
#include "zero_copy_resource.h"

#include <a4/io/A4Stream.pb.h>

namespace a4{ namespace io{

    class InputStreamImpl
    {
        public:
            InputStreamImpl(unique<ZeroCopyStreamResource>, std::string name);
            virtual ~InputStreamImpl();

            /// Returns the next regular message in the stream.
            shared<A4Message> next(bool skip_metadata=true);
            /// Returns the next bare message in the stream.
            shared<A4Message> next_bare_message();
            /// Returns the next regular or metadata message in the stream.
            shared<A4Message> next_with_metadata() { return next(false); }
            /// Return the current metadata message.
            shared<const A4Message> current_metadata() {return _current_metadata; }
            /// Seek to the given header/metadata combination.
            /// If carry==false, specifying a metadata index not in that header section
            /// causes an exception, otherwise the next header is used, or false is returned on EOF.
            bool seek_to(uint32_t header, uint32_t metadata, bool carry=false);
            /// Skip to the start of the next metadata block. Return false if EOF, true if not.
            bool skip_to_next_metadata() { return seek_to(_current_header_index, _current_metadata_index+1, true); }
            /// True if new metadata has appeared since the last call to this function.
            bool new_metadata() { 
                if (!_started) startup();
                if (_new_metadata) { _new_metadata = false; return true; } else return false;
            }
            /// True if the stream has not ended or encountered an error.
            bool good() { return _good; }
            /// True if the stream has encountered an error.
            bool error() { return _error; }
            /// True if the stream has finished without error.
            bool end() { return !_error && !_good; }
            /// explicitely close the stream
            void close() {
                _coded_in.reset();
                _compressed_in.reset();
                _raw_in.reset();
                _good = false;
            };
            
            size_t ByteCount() { return _raw_in->ByteCount(); }

            std::string str() { return _inputname; };
            
            const std::vector<std::vector<shared<a4::io::A4Message>>>& all_metadata() {
                if (_metadata_per_header.size() == 0) {
                    if (_started)
                        FATAL("Coding Bug: all_metadata first called after reading started!");
                    startup(true);
                }
                return _metadata_per_header;
            }
            
            const std::vector<StreamFooter>& footers() {
                if (_footers.size() == 0) {
                    if (_started) 
                        FATAL("Coding Bug: footers() first called after reading started!");
                    startup(true);
                }
                return _footers;
            }

        private:
            unique<ZeroCopyStreamResource> _raw_in;
            unique<BaseCompressedInputStream> _compressed_in;
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
            shared<A4Message> _current_metadata;
            std::vector<std::vector<uint64_t>> _metadata_offset_per_header;
            std::vector<std::vector<shared<A4Message>>> _metadata_per_header;
            std::vector<bool> _headers_forward;
            std::vector<StreamFooter> _footers;
    
            // internal functions
            void startup(bool discovery_requested=false);
            void reset_coded_stream();
            bool discover_all_metadata();
            bool start_compression(const a4::io::StartCompressedSection& cs);
            bool stop_compression(const a4::io::EndCompressedSection& cs);
            void drop_compression();
            bool read_header(bool discovery_requested=false);
            int64_t seek(int64_t position);
            int64_t seek_back(int64_t position);
            shared<A4Message> bare_message();
            shared<A4Message> next_message();
            bool handle_compressed_section(shared<A4Message> msg);
            bool handle_stream_command(shared<A4Message> msg);
            bool handle_metadata(shared<A4Message> msg);
            bool carry_metadata(uint32_t& header, uint32_t& metadata);

            void notify_last_unread_message();
            shared<A4Message> _last_unread_message;

            // set error/end status and return A4Message
            bool set_error();
            bool set_end();
    };

};};

#endif
