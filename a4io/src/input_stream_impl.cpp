#define _FILE_OFFSET_BITS 64

#include <deque>
#include <iostream>
#include <tuple>

#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <boost/function.hpp>
using boost::function;

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
typedef boost::unique_lock<boost::mutex> Lock;

#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <a4/config.h>
#include <a4/types.h>

#include <a4/dynamic_message.h>
#include "gzip_stream.h"
#ifdef HAVE_SNAPPY
#include "snappy_stream.h"
#endif
#include "zero_copy_resource.h"
#include "input_stream_impl.h"
#include <a4/io/A4Stream.pb.h>
#include <a4/input_stream.h>

using std::string;

using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::FileInputStream;
using google::protobuf::io::CodedInputStream;

using google::protobuf::DynamicMessageFactory;
using google::protobuf::SimpleDescriptorDatabase;
using google::protobuf::DescriptorPool;

using google::protobuf::FileDescriptorProto;
using google::protobuf::FileDescriptor;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Reflection;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const int START_MAGIC_len = 8;
const int END_MAGIC_len = 8;
const uint32_t HIGH_BIT = 1 << 31;

namespace a4{ 
namespace io{

InputStreamImpl::InputStreamImpl(UNIQUE<ZeroCopyStreamResource> in, 
                                 std::string name) {
    _started = false;
    _raw_in = std::move(in);
    _inputname = name;
    _compressed_in.reset();
    _coded_in.reset();
    _good = true;
    _error = false;
    _new_metadata = false;
    _discovery_complete = false;
    _items_read = 0;
    _current_metadata_refers_forward = false;
    _current_header_index = 0;
    _current_metadata_index = 0;
    _last_unread_message.reset();
    _do_reset_metadata = false;
    _hint_copy = false;
}

InputStreamImpl::~InputStreamImpl() {};

bool InputStreamImpl::set_error() {
    _error = true;
    _good = false;
    return false;
}

bool InputStreamImpl::set_end() {
    _good = false;
    return false;
}

bool InputStreamImpl::set_hint_copy(bool do_copy) {
    _hint_copy = do_copy;
    if (not _hint_copy) {
        notify_last_unread_message();
    }
}

void InputStreamImpl::startup(bool discovery_requested) {
    // Initialize to defined state
    _started = true;
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    
    // Push limit of read bytes
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));

    if (!read_header(discovery_requested)) {
        if(_error) {
            ERROR("Header corrupted!");
        } else {
            ERROR("File empty!");
        }
        
        set_error();
        return;
    }
    _current_header_index = 0;
}

bool InputStreamImpl::read_header(bool discovery_requested)
{
    if (_hint_copy) notify_last_unread_message();
    // Note: in the following i use that bool(set_end()) == false 
    // && bool(set_error()) == false
    string magic;
    if (!_coded_in->ReadString(&magic, 8))
        return set_end();

    if (0 != magic.compare(START_MAGIC))
        return set_error();

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size))
        return set_error();

    uint32_t message_type = 0;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type))
            return set_error();
    }
    if (!message_type == _fixed_class_id<StreamHeader>())
        return set_error();

    StreamHeader h;
    CodedInputStream::Limit lim = _coded_in->PushLimit(size);
    if (!h.ParseFromCodedStream(_coded_in.get()))
        return set_error();
    _coded_in->PopLimit(lim);
    
    if (h.a4_version() != 2) {
        ERROR("Unknown A4 stream version (", h.a4_version(), ")");
        return set_error();
    }

    _current_metadata_refers_forward = h.metadata_refers_forward();
    _current_class_pool.reset(new ProtoClassPool());
    if (!_discovery_complete) {
        if (!_current_metadata_refers_forward) {
            if (!_raw_in->seekable()) {
                ERROR("Cannot read reverse metadata from non-seekable stream!");
                return set_error();
            } else if (!discover_all_metadata()) {
                ERROR("Failed to discover metadata - file corrupted?");
                return set_error();
            }

            _current_metadata_index = 0;
            if (_metadata_per_header[_current_header_index].size() > 0) {
                _current_metadata = _metadata_per_header[_current_header_index][0];
            }
        } else {
            _current_metadata_index = -1;
            if (discovery_requested and not discover_all_metadata()) {
                ERROR("a4::io:InputStreamImpl - Failed to discover metadata - file corrupted?");
                return set_error();
            }
        }
    }
    _new_metadata = true; // always true, it could be <no metadata>
    return true;
}

bool InputStreamImpl::discover_all_metadata() {
    if (_hint_copy) notify_last_unread_message();
    assert(_metadata_per_header.size() == 0);
    // Temporary ProtoClassPool for reading static messages
    shared<ProtoClassPool> temp_pool(new ProtoClassPool());
    unsigned int _temp_header_index = _current_header_index;
    _current_header_index = 0;

    int64_t size = 0;
    std::deque<uint64_t> headers;
    std::deque<bool> _temp_headers_forward;
    std::deque<std::vector<shared<A4Message>>> _temp_metadata_per_header;
    std::deque<std::vector<uint64_t>>  _temp_metadata_offset_per_header;

    while (true) {
        if (seek_back(-size - END_MAGIC_len) == -1)
            return false;
        
        string magic;
        if (!_coded_in->ReadString(&magic, 8)) {
            ERROR("Unexpected EOF during metadata scan");
            return false;
        }
        
        if (seek_back(-size - END_MAGIC_len - 4) == -1)
            return false;
        
        uint32_t footer_size = 0;
        if (!_coded_in->ReadLittleEndian32(&footer_size))
            return false;
        
        // Seek to footer
        uint32_t footer_msgsize  = END_MAGIC_len + 4 + footer_size + 8;
        uint64_t footer_start = - size - footer_msgsize;
        int64_t footer_abs_start = seek_back(footer_start);
        if (footer_abs_start == -1)
            return false;
        
        shared<A4Message> msg = next_message();
        if (!msg->is<StreamFooter>()) {
            ERROR("Unknown footer class!");
            return false;
        }
        
        const StreamFooter* footer = msg->as<StreamFooter>();
        _footers.push_back(*footer);
            
        size += footer->size() + footer_msgsize;
        
        // Read all ProtoClasses associated with this footer
        _current_class_pool.reset(new ProtoClassPool());
        foreach(uint64_t offset, footer->protoclass_offsets()) {
            uint64_t metadata_start = footer_abs_start - footer->size() + offset;
            if (seek(metadata_start) == -1) 
                return false;
                
            shared<A4Message> msg = next_message();
            drop_compression();
            
            const ProtoClass* proto = msg->as<ProtoClass>();
            assert(proto);
            _current_class_pool->add_protoclass(*proto);
        }
        
        // Populate the class_name on the ClassCount
        foreach (auto& cc, *_footers.back().mutable_class_count())
            cc.set_class_name(_current_class_pool->descriptor(cc.class_id())->full_name());

        // Read all metadata associated with this footer
        std::vector<shared<A4Message>> _this_headers_metadata;
        std::vector<uint64_t> _this_headers_metadata_offsets;
        foreach(uint64_t offset, footer->metadata_offsets()) {
            uint64_t metadata_start = footer_abs_start - footer->size() + offset;
            _this_headers_metadata_offsets.push_back(metadata_start);
            if (seek(metadata_start) == -1) 
                return false;
                
            shared<A4Message> msg = next_message();
            drop_compression();
            _this_headers_metadata.push_back(msg);
        }
        _temp_metadata_per_header.push_front(_this_headers_metadata);
        _temp_metadata_offset_per_header.push_front(_this_headers_metadata_offsets);

        // Jump to next footer
        int64_t tell = seek_back(-size);
        headers.push_front(tell);
        if (tell == -1)
            return false;      

        seek(tell + START_MAGIC_len);
        shared<A4Message> hmsg = next_message();
        drop_compression();
        if (!hmsg->is<StreamHeader>()) {
            ERROR("a4::io:InputStreamImpl - Unknown header class!");
            return false;
        }
        const StreamHeader* header = hmsg->as<StreamHeader>();
        _temp_headers_forward.push_back(header->metadata_refers_forward());

        if (tell == 0)
            break;
    }
    
    // Seek back to original header
    seek(headers[_temp_header_index] + START_MAGIC_len);
    next_message(); // read the header again
    _discovery_complete = true;
    _metadata_per_header.insert(
        _metadata_per_header.end(),
        _temp_metadata_per_header.begin(),
        _temp_metadata_per_header.end());
    _metadata_offset_per_header.insert(
        _metadata_offset_per_header.end(),
        _temp_metadata_offset_per_header.begin(),
        _temp_metadata_offset_per_header.end());
    
    _current_header_index = _temp_header_index;
    _headers_forward.insert(_headers_forward.end(), _temp_headers_forward.begin(), _temp_headers_forward.end());
    return true;
}

int64_t InputStreamImpl::seek_back(int64_t position) {
    assert(!_compressed_in);
    if (_hint_copy) notify_last_unread_message();
    _coded_in.reset();
    if (!_raw_in->SeekBack(-position))
        return -1;
    int64_t pos = _raw_in->Tell();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return pos;
};

int64_t InputStreamImpl::seek(int64_t position) {
    assert(!_compressed_in);
    if (_hint_copy) notify_last_unread_message();
    _coded_in.reset();
    if (!_raw_in->Seek(position))
        return -1;
    int64_t pos = _raw_in->Tell();
    
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return pos;
}

bool InputStreamImpl::carry_metadata(uint32_t& header, int32_t& metadata) {
    if ((0 < header) or not (header < _metadata_offset_per_header.size()))
        return false;
    while (metadata < 0 and header > 0) {
        header -= 1;
        metadata += _metadata_offset_per_header[header].size();
    }
    while (header < _metadata_offset_per_header.size() 
        and metadata > static_cast<int32_t>(_metadata_offset_per_header[header].size())) {
        metadata -= _metadata_offset_per_header[header].size();
        header += 1;
    }
    if ((0 < header) or not (header < _metadata_offset_per_header.size()))
        return false;
    return true;
}

bool InputStreamImpl::seek_to(uint32_t header, int32_t metadata, bool carry) {
    drop_compression();
    if (!_discovery_complete) {
        if (seek(0) == -1) {
            ERROR("a4::io:InputStreamImpl - Cannot skip in this unseekable stream!");
            set_error();
            return false;
        }
        if (not discover_all_metadata()) {
            ERROR("a4::io:InputStreamImpl - Failed to discover metadata - file corrupted?");
            set_error();
            return false;
        }
    }
    if (carry and not carry_metadata(header, metadata)) { // modifies header and metadata
        if (not carry) {
            ERROR("a4::io:InputStreamImpl - Attempt to seek to nonexistent metadata!");
        }
        return false;
    }

    if (_headers_forward[header]) {
        // If the metadata refers forward, just seek to it
        _current_header_index = header;
        // will be incremented when next metadata is read
        _current_metadata_index = metadata - 1;
        if (metadata == static_cast<int32_t>(_metadata_offset_per_header[header].size())) {
            // No more metadata in this header, current position is end
            // of stream.
            set_end();
            return false;
        }
        seek(_metadata_offset_per_header[header][metadata]);
    } else {
        if (metadata == 0 && header == 0) {
            // Easy case
            _current_header_index = 0;
            _current_metadata_index = 0;
            seek(0);
        } else {
            // More complicated - find previous metadata, seek to that, and skip it
            metadata -= 1;
            carry_metadata(header, metadata); // modifies header and metadata
            _current_header_index = header;
            _current_metadata_index = metadata;
            if (metadata == static_cast<int32_t>(_metadata_offset_per_header[header].size())) {
                // No more metadata in this header, current position is end
                // of stream.
                set_end();
                return false;
            }
            seek(_metadata_offset_per_header[header][metadata]);
            next(false); // read only the next metadata    
        }
    }
    return true;
}

bool InputStreamImpl::start_compression(const StartCompressedSection& cs) {
    assert(!_compressed_in);
    if (_hint_copy) notify_last_unread_message();
    _coded_in.reset();

    if (cs.compression() == StartCompressedSection_Compression_ZLIB) {
        _compressed_in.reset(new GzipInputStream(_raw_in.get(), GzipInputStream::ZLIB));
    } else if (cs.compression() == StartCompressedSection_Compression_GZIP) {
        _compressed_in.reset(new GzipInputStream(_raw_in.get(), GzipInputStream::GZIP));
    } else if (cs.compression() == StartCompressedSection_Compression_SNAPPY) {
#ifdef HAVE_SNAPPY
        _compressed_in.reset(new SnappyInputStream(_raw_in.get()));
#else
        FATAL("This file uses compression by the 'Snappy' library, "
              "which was not compiled in!");
#endif
    } else {
        ERROR("Unknown compression type: ", cs.compression());
        return false;
    }

    _coded_in.reset(new CodedInputStream(_compressed_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return true;
}

void InputStreamImpl::drop_compression() {
    if(!_compressed_in) 
        return;
        
    if (_hint_copy) notify_last_unread_message();
    _coded_in.reset();
    _compressed_in.reset();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
}

bool InputStreamImpl::stop_compression(const EndCompressedSection& cs) {
    assert(_compressed_in);
    if (_hint_copy) notify_last_unread_message();
    _coded_in.reset();
    if (!_compressed_in->ExpectAtEnd()) {
        ERROR("Compressed section did not end where it should");
        return false;
    }
    _compressed_in.reset();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    return true;
}

void InputStreamImpl::reset_coded_stream() {
    if (_hint_copy) notify_last_unread_message();
    _coded_in.reset();
    if (_compressed_in) {
        _coded_in.reset(new CodedInputStream(_compressed_in.get()));
    } else {
        _coded_in.reset(new CodedInputStream(_raw_in.get()));
    }
    _coded_in->SetTotalBytesLimit(pow(1024, 3), pow(1024, 3));
}

shared<A4Message> InputStreamImpl::bare_message() {
    if (!_started) 
        startup();
    if (!_good) 
        return shared<A4Message>();

    notify_last_unread_message();

    if (_items_read++ % 100 == 0) {
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

    shared<A4Message> umsg(new A4Message(class_id, size, _coded_in, _current_class_pool));
    _last_unread_message = umsg;
    return umsg;
}

/// Deals with a4.io.StartCompressedSection and a4.io.EndCompressedSection messages
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
bool InputStreamImpl::handle_stream_command(shared<A4Message> msg) {
    if (msg->class_id() < 100 || msg->class_id() > 200) 
        return false;
    
    if (msg->is<StreamFooter>()) {
        notify_last_unread_message();
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

shared<A4Message> InputStreamImpl::next(bool skip_metadata) {
    shared<A4Message> msg = next_message();
    if (msg and handle_stream_command(msg)) 
        return next(skip_metadata);


    if (msg and handle_metadata(msg) && skip_metadata) 
        return next(skip_metadata);
    return msg;
}

shared<A4Message> InputStreamImpl::next_bare_message() {
    shared<A4Message> msg = next_message();
    if (msg and handle_stream_command(msg))
        return msg;
    if (msg and handle_metadata(msg))
        return msg;
    return msg;
}

void InputStreamImpl::notify_last_unread_message() {
    if (_last_unread_message) {
        _last_unread_message->invalidate_stream();
        _last_unread_message.reset();
    }
}

};}; // namespace a4::io
