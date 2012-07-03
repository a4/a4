#ifndef _A4_INPUT_STREAM_IMPL_
#define _A4_INPUT_STREAM_IMPL_

#include <tuple>
#include <unordered_set>
#include <a4/types.h>

#include "base_compressed_streams.h"
#include "proto_class_pool.h"
#include "zero_copy_resource.h"

#include <a4/io/A4Stream.pb.h>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
typedef boost::unique_lock<boost::mutex> Lock;

using std::string;
const uint32_t HIGH_BIT = 1 << 31;
const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const int START_MAGIC_len = 8;
const int END_MAGIC_len = 8;

namespace a4{ namespace io{

    class InputStreamImpl
    {
        public:
            InputStreamImpl(UNIQUE<ZeroCopyStreamResource>, std::string name);
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
            bool seek_to(uint32_t header, int32_t metadata, bool carry=false);
            /// Skip to the start of the next metadata block. Return false if EOF, true if not.
            bool skip_to_next_metadata() {
                return seek_to(_current_header_index, _current_metadata_index+1, true);
            }
            /// True if new metadata has appeared since the last call to this function.
            bool new_metadata() { 
                if (!_started) startup();
                if (_new_metadata) {
                    _new_metadata = false;
                    return true;
                }
                return false;
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
            
            std::vector<const google::protobuf::FileDescriptor*> get_filedescriptors() {
                if (not _current_class_pool) {
                    if (_started) 
                        FATAL("Coding Bug: footers() first called after reading started!");
                    startup(true);
                }
                return _current_class_pool->get_filedescriptors();
            }

            void set_hint_copy(bool hint_copy);
            bool try_read(Message & msg, const google::protobuf::Descriptor* d);

        private:
            UNIQUE<ZeroCopyStreamResource> _raw_in;
            UNIQUE<BaseCompressedInputStream> _compressed_in;
            shared<google::protobuf::io::CodedInputStream> _coded_in;
            shared<ProtoClassPool> _current_class_pool;

            // variables set at construction time
            std::string _inputname;

            // status variables
            bool _good, _error, _started, _discovery_complete, _do_reset_metadata;
            uint64_t _items_read;
            unsigned int _current_header_index;
            int32_t _current_metadata_index;
            int _fileno;

            // metadata-related status
            bool _new_metadata, _current_metadata_refers_forward;
            shared<A4Message> _current_metadata;
            shared<A4Message> _pickup;
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
            bool carry_metadata(uint32_t& header, int32_t& metadata);

            void notify_last_unread_message();
            shared<A4Message> _last_unread_message;

            // set error/end status and return A4Message
            bool set_error();
            bool set_end();

            bool _hint_copy;
    };

inline
bool InputStreamImpl::try_read(Message & msg, const google::protobuf::Descriptor* d) {
    if (!_started) 
        startup();
    if (!_good) 
        return false;
    if (_hint_copy) notify_last_unread_message();
    if (_items_read++ % 10000 == 0) {
        reset_coded_stream();
    }

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size)) {
        if (_compressed_in && _compressed_in->ByteCount() == 0) {
            FATAL("Reading from compressed section failed!");
        } else {
            FATAL("Unexpected end of file or corruption [0]!");
        }
    }

    uint32_t class_id = 0;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&class_id))
            FATAL("Unexpected end of file [1]!");
    }
    if (_current_class_pool->check_match(class_id, d)) {
        auto lim = _coded_in->PushLimit(size);
        if (not msg.ParseFromCodedStream(_coded_in.get())) {
            FATAL("Failed to read expected event!");
        }
        _coded_in->PopLimit(lim);
        return true;
    } else {
        auto _message = _current_class_pool->parse_message(class_id, _coded_in, size);
        _pickup.reset(new A4Message(class_id, _message, _current_class_pool));
        return false;
    }
}

inline
shared<A4Message> InputStreamImpl::bare_message() {
    if (_pickup) {
        auto res = _pickup;
        _pickup.reset();
        return res;
    }

    if (!_started) 
        startup();
    if (!_good) 
        return shared<A4Message>();

    if (_hint_copy) notify_last_unread_message();

    if (_items_read++ % 10000 == 0) {
        reset_coded_stream();
    }

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size)) {
        if (_compressed_in && _compressed_in->ByteCount() == 0) {
            FATAL("Reading from compressed section failed! inside compression: ",
                  bool(_compressed_in), " compressed bytecount: ",
                  _compressed_in ? _compressed_in->ByteCount() : 0,
                  " raw_in bytecount: ", _raw_in->ByteCount());
        } else {
            FATAL("Unexpected end of file or corruption [0]!");
        }
    }

    uint32_t class_id = 0;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&class_id))
            FATAL("Unexpected end of file [1]!");
    }
    
    //VERBOSE("Next part: ", _raw_in->ByteCount(), " -- ", size, " - ", class_id);

    if (_hint_copy) {

        shared<A4Message> umsg(new A4Message(class_id, size, _coded_in, _current_class_pool));
        _last_unread_message = umsg;
        return umsg;
    } else {
        auto _message = _current_class_pool->parse_message(class_id, _coded_in, size);
        return shared<A4Message>(new A4Message(class_id, _message, _current_class_pool));
    }
}

/// Deals with a4.io.StartCompressedSection and a4.io.EndCompressedSection messages
inline
bool InputStreamImpl::handle_compressed_section(shared<A4Message> msg) {
    if (msg->is<StartCompressedSection>()) {
        if (!start_compression(*msg->as<StartCompressedSection>()))
            FATAL("Unable to start compressed section!");
        return true;
    } else if (msg->is<EndCompressedSection>()) {
        if (!stop_compression(*msg->as<EndCompressedSection>()))
            FATAL("Unable to stop compressed section!");
        return true;
    }
    return false;
}

/// Deals with all internal messages, with class id between 100 and 200.
inline
bool InputStreamImpl::handle_stream_command(shared<A4Message> msg) {
    if (msg->class_id() < 100 || msg->class_id() > 200) 
        return false;
    
    if (msg->is<StreamFooter>()) {
        if (_hint_copy) notify_last_unread_message();
        uint32_t size;
        if (!_coded_in->ReadLittleEndian32(&size))
            FATAL("Unexpected end of file [3]!");
        
        string magic;
        if (!_coded_in->ReadString(&magic, 8))
            FATAL("Unexpected end of file [4]!");
            
        if (0 != magic.compare(END_MAGIC))
            FATAL("Corrupt footer! Read: ", magic);
        
        if (_coded_in->ExpectAtEnd()) {
             // Regular end of stream
            _good = false;
            return true;
        }
        _current_header_index++;
        if (!read_header()) {
            if (_error)
                FATAL("Corrupt header!");
            _good = false; // Slightly strange but regular end of stream
            return true;
        }
        _current_metadata = shared<A4Message>();
        if (!_current_metadata_refers_forward) {
            _current_metadata_index = 0;
            if (_metadata_per_header[_current_header_index].size() > 0)
                _current_metadata = _metadata_per_header[_current_header_index][0];
        } else {
            _current_metadata_index = -1;
        }
        _do_reset_metadata = false; // if we had an increment before, ignore it
        _new_metadata = true; // a footer invalidates metadata
        return true;
    } else if (msg->is<StreamHeader>()) {
        FATAL("Unexpected header!");
    } else if (msg->is<ProtoClass>()) {
        _current_class_pool->add_protoclass(*msg->as<ProtoClass>());
        return true;
    }
    
    {
        static Lock l;
        
        static std::unordered_set<uint32_t> warned_ids;
        const auto id = msg->class_id();
        
        if (warned_ids.find(id) != warned_ids.end()) {
            warned_ids.insert(id);
            WARNING("Encountered unexpected internal class id: ", id, 
                    ". The input may be from a newer version of A4? "
                    "Continuing anyway..");
        }
    }
    return false;
}

inline
bool InputStreamImpl::handle_metadata(shared<A4Message> msg) {
    //if (_current_header_index > 0) {
    //    for (int i = 0; i < _current_header_index; i++) {
    //        _metadata_per_header[i].clear();
    //    }
    //}
    if (msg->metadata()) {
        _current_metadata_index++;
        if (_current_metadata_refers_forward) {
            _current_metadata = msg;
            _new_metadata = true;
        } else {
            _do_reset_metadata = true;
        }
        return true;
    }
    return false;
}

inline
shared<A4Message> InputStreamImpl::next_message() {
    if (_do_reset_metadata) {
        _do_reset_metadata = false;
        _current_metadata = shared<A4Message>();
        if (_metadata_per_header.size() > _current_header_index) {
            auto& header_metadata = _metadata_per_header[_current_header_index];
            if (static_cast<int32_t>(header_metadata.size()) > _current_metadata_index)
                _current_metadata = header_metadata[_current_metadata_index];
        }
        _new_metadata = true;
    }
    shared<A4Message> msg = bare_message();
    if (msg and handle_compressed_section(msg))
        return next_message();
    return msg;
}

inline
shared<A4Message> InputStreamImpl::next(bool skip_metadata) {
    shared<A4Message> msg = next_message();
    if (msg and handle_stream_command(msg)) 
        return next(skip_metadata);


    if (msg and handle_metadata(msg) && skip_metadata) 
        return next(skip_metadata);
    return msg;
}

inline
shared<A4Message> InputStreamImpl::next_bare_message() {
    shared<A4Message> msg = next_message();
    if (msg and handle_stream_command(msg))
        return msg;
    if (msg and handle_metadata(msg))
        return msg;
    return msg;
}

inline
void InputStreamImpl::notify_last_unread_message() {
    if (_last_unread_message) {
        _last_unread_message->invalidate_stream();
        _last_unread_message.reset();
    }
}


};};

#endif
