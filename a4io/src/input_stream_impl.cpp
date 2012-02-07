#define _FILE_OFFSET_BITS 64

#include <a4/config.h>
#include <a4/types.h>

#include <iostream>
#include <tuple>

#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <boost/function.hpp>

using boost::function;

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

#include "dynamic_message.h"
#include "gzip_stream.h"
#ifdef HAVE_SNAPPY
#include "snappy_stream.h"
#endif
#include "zero_copy_resource.h"
#include "input_stream_impl.h"
#include "a4/io/A4Stream.pb.h"
#include "a4/input_stream.h"

using std::string;
using std::cerr;
using std::endl;
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
const uint32_t HIGH_BIT = 1<<31;

namespace a4{ namespace io{

InputStreamImpl::InputStreamImpl(unique<ZeroCopyStreamResource> in, std::string name) {
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
}

InputStreamImpl::~InputStreamImpl() {};

A4Message InputStreamImpl::set_error() {
    _error = true;
    _good = false;
    return A4Message();
}

A4Message InputStreamImpl::set_end() {
    _good = false;
    return A4Message();
}

void InputStreamImpl::startup(bool discovery_requested) {
    // Initialize to defined state
    _started = true;
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    // Push limit of read bytes
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));

    if (!read_header(discovery_requested)) {
        if(_error) std::cerr << "ERROR - a4::io:InputStreamImpl - Header corrupted!" << std::endl;
        else std::cerr << "ERROR - a4::io:InputStreamImpl - File empty!" << std::endl;
        set_error();
        return;
    }
    _current_header_index = 0;
}

bool InputStreamImpl::read_header(bool discovery_requested)
{
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
        std::cerr << "ERROR - a4::io:InputStreamImpl - Unknown A4 stream version (";
        std::cerr << h.a4_version() << ")" << std::endl;
        return set_error();
    }

    _current_metadata_refers_forward = h.metadata_refers_forward();
    _current_class_pool.reset(new ProtoClassPool());
    if (!_discovery_complete) {
        if (!_current_metadata_refers_forward) {
            if (!_raw_in->seekable()) {
                std::cerr << "ERROR - a4::io:InputStreamImpl - Cannot read reverse metadata from non-seekable stream!" << std::endl;
                return set_error();
            } else if (!discover_all_metadata()) {
                std::cerr << "ERROR - a4::io:InputStreamImpl - Failed to discover metadata - file corrupted?" << std::endl;
                return set_error();
            }

            _current_metadata_index = 0;
            if (_metadata_per_header[_current_header_index].size() > 0) {
                _current_metadata = _metadata_per_header[_current_header_index][0];
                _new_metadata = true;
            }
        } else {
            _current_metadata_index = -1;
            if (discovery_requested and not discover_all_metadata()) {
                std::cerr << "ERROR - a4::io:InputStreamImpl - Failed to discover metadata - file corrupted?" << std::endl;
                return set_error();
            }
        }
    }
    return true;
}

bool InputStreamImpl::discover_all_metadata() {
    assert(_metadata_per_header.size() == 0);
    // Temporary ProtoClassPool for reading static messages
    shared<ProtoClassPool> temp_pool(new ProtoClassPool());
    unsigned int _temp_header_index = _current_header_index;
    _current_header_index = 0;

    int64_t size = 0;
    std::deque<uint64_t> headers;
    std::deque<bool> _temp_headers_forward;
    std::deque<std::vector<A4Message>> _temp_metadata_per_header;
    std::deque<std::vector<uint64_t>>  _temp_metadata_offset_per_header;

    while (true) {
        if (seek_back(-size - END_MAGIC_len) == -1) return false;
        string magic;
        if (!_coded_in->ReadString(&magic, 8)) {
            std::cerr << "ERROR - a4::io:InputStreamImpl - Unexpected end of file during scan!" << std::endl; 
            return false;
        }
        if (seek_back(-size - END_MAGIC_len - 4) == -1) return false;



        uint32_t footer_size = 0;
        if (!_coded_in->ReadLittleEndian32(&footer_size)) return false;
        uint32_t footer_msgsize  = END_MAGIC_len + 4 + footer_size + 8;
        uint64_t footer_start = - size - footer_msgsize;
        int64_t footer_abs_start = seek_back(footer_start);
        if (footer_abs_start == -1) return false;
        A4Message msg = next_message();
        if (!msg.is<StreamFooter>()) {
            std::cerr << "ERROR - a4::io:InputStreamImpl - Unknown footer class!" << std::endl;
            return false;
        }
        shared<StreamFooter> footer = msg.as<StreamFooter>();
        size += footer->size() + footer_msgsize;

        _current_class_pool.reset(new ProtoClassPool());
        foreach(uint64_t offset, footer->protoclass_offsets()) {
            uint64_t metadata_start = footer_abs_start - footer->size() + offset;
            if (seek(metadata_start) == -1) return false;
            A4Message msg = next_message();
            drop_compression();
            shared<ProtoClass> proto = msg.as<ProtoClass>();
            assert(proto);
            _current_class_pool->add_protoclass(*proto);
        }

        std::vector<A4Message> _this_headers_metadata;
        std::vector<uint64_t> _this_headers_metadata_offsets;
        foreach(uint64_t offset, footer->metadata_offsets()) {
            uint64_t metadata_start = footer_abs_start - footer->size() + offset;
            _this_headers_metadata_offsets.push_back(metadata_start);
            if (seek(metadata_start) == -1) return false;
            A4Message msg = next_message();
            drop_compression();
            _this_headers_metadata.push_back(msg);
        }
        _temp_metadata_per_header.push_front(_this_headers_metadata);
        _temp_metadata_offset_per_header.push_front(_this_headers_metadata_offsets);

        int64_t tell = seek_back(-size);
        headers.push_front(tell);
        if (tell == -1) return false;      

        seek(tell + START_MAGIC_len);
        A4Message hmsg = next_message();
        drop_compression();
        if (!hmsg.is<StreamHeader>()) {
            std::cerr << "ERROR - a4::io:InputStreamImpl - Unknown header class!" << std::endl;
            return false;
        }
        shared<StreamHeader> header = hmsg.as<StreamHeader>();
        _temp_headers_forward.push_back(header->metadata_refers_forward());

        if (tell == 0) break;
    }
    // Seek back to original header
    seek(headers[_temp_header_index] + START_MAGIC_len);
    next_message(); // read the header again
    _discovery_complete = true;
    _metadata_per_header.insert(_metadata_per_header.end(), _temp_metadata_per_header.begin(), _temp_metadata_per_header.end());
    _metadata_offset_per_header.insert(_metadata_offset_per_header.end(),
        _temp_metadata_offset_per_header.begin(),
        _temp_metadata_offset_per_header.end());
    _current_header_index = _temp_header_index;
    _headers_forward.insert(_headers_forward.end(), _temp_headers_forward.begin(), _temp_headers_forward.end());
    return true;
}

int64_t InputStreamImpl::seek_back(int64_t position) {
    assert(!_compressed_in);
    _coded_in.reset();
    if (!_raw_in->SeekBack(-position)) return -1;
    int64_t pos = _raw_in->Tell();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return pos;
};

int64_t InputStreamImpl::seek(int64_t position) {
    assert(!_compressed_in);
    _coded_in.reset();
    if (!_raw_in->Seek(position)) return -1;
    int64_t pos = _raw_in->Tell();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return pos;
};

bool InputStreamImpl::carry_metadata(uint32_t& header, uint32_t& metadata) {
    if ((0 < header) or not (header < _metadata_offset_per_header.size())) return false;
    while (metadata < 0 and header > 0) {
        header -= 1;
        metadata += _metadata_offset_per_header[header].size();
    }
    while (header < _metadata_offset_per_header.size() 
        and metadata > _metadata_offset_per_header[header].size()) {
        metadata -= _metadata_offset_per_header[header].size();
        header += 1;
    }
    if ((0 < header) or not (header < _metadata_offset_per_header.size())) return false;
    return true;
}

bool InputStreamImpl::seek_to(uint32_t header, uint32_t metadata, bool carry) {
    drop_compression();
    if (!_discovery_complete) {
        if (seek(0) == -1) {
            std::cerr << "ERROR - a4::io:InputStreamImpl - Cannot skip in this unseekable stream!" << std::endl;    
            set_error();
            return false;
        }
        if (not discover_all_metadata()) {
            std::cerr << "ERROR - a4::io:InputStreamImpl - Failed to discover metadata - file corrupted?" << std::endl;
            set_error();
            return false;
        }
    }
    if (carry and not carry_metadata(header, metadata)) { // modifies header and metadata
        if (not carry) {
            std::cerr << "ERROR - a4::io:InputStreamImpl - Attempt to seek to nonexistent metadata!" << std::endl;
        }
        return false;
    }

    if (_headers_forward[header]) {
        // If the metadata refers forward, just seek to it
        _current_header_index = header;
        _current_metadata_index = metadata - 1; // will be incremented when next metadata is read
        seek(_metadata_offset_per_header[header][metadata]);
    } else {
        // More complicated - find previous metadata, seek to that, and skip it
        if (metadata == 0 && header == 0) {
            // Easy case
            _current_header_index = 0;
            _current_metadata_index = 0;
            seek(0);
        } else {
            metadata -= 1;
            carry_metadata(header, metadata); // modifies header and metadata
            _current_header_index = header;
            _current_metadata_index = metadata;
            seek(_metadata_offset_per_header[header][metadata]);
            next(false); // read only the next metadata    
        }
    }
    return true;
}

bool InputStreamImpl::start_compression(const StartCompressedSection& cs) {
    assert(!_compressed_in);
    _coded_in.reset();

    if (cs.compression() == StartCompressedSection_Compression_ZLIB) {
        _compressed_in.reset(new GzipInputStream(_raw_in.get(), GzipInputStream::ZLIB));
    } else if (cs.compression() == StartCompressedSection_Compression_GZIP) {
        _compressed_in.reset(new GzipInputStream(_raw_in.get(), GzipInputStream::GZIP));
    } else if (cs.compression() == StartCompressedSection_Compression_SNAPPY) {
#ifdef HAVE_SNAPPY
        _compressed_in.reset(new SnappyInputStream(_raw_in.get()));
#else
        throw a4::Fatal("This file uses compression by the 'Snappy' library, which was not compiled in!");
#endif
    } else {
        std::cerr << "ERROR - a4::io:InputStreamImpl - Unknown compression type " << cs.compression() << std::endl;
        return false;
    }

    _coded_in.reset(new CodedInputStream(_compressed_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return true;
}

void InputStreamImpl::drop_compression() {
    if(!_compressed_in) return;
    _coded_in.reset();
    _compressed_in.reset();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
}

bool InputStreamImpl::stop_compression(const EndCompressedSection& cs) {
    assert(_compressed_in);
    _coded_in.reset();
    if (!_compressed_in->ExpectAtEnd()) {
        std::cerr << "ERROR - a4::io:InputStreamImpl - Compressed section did not end where it should!" << std::endl;
        return false;
    }
    _compressed_in.reset();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    return true;
}

void InputStreamImpl::reset_coded_stream() {
    _coded_in.reset();
    if (_compressed_in) {
        _coded_in.reset(new CodedInputStream(_compressed_in.get()));
    } else {
        _coded_in.reset(new CodedInputStream(_raw_in.get()));
    }
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
}

A4Message InputStreamImpl::bare_message() {
    if (!_started) startup();
    if (!_good) return A4Message();

    if (_items_read++ % 100 == 0) reset_coded_stream();

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size)) {
        if (_compressed_in && _compressed_in->ByteCount() == 0) {
            throw a4::Fatal("a4::io:InputStreamImpl - Reading from compressed section failed!");
        } else {
            throw a4::Fatal("a4::io:InputStreamImpl - Unexpected end of file or corruption [0]!");
        }
    }

    uint32_t class_id = 0;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&class_id)) {
            throw a4::Fatal("a4::io:InputStreamImpl - Unexpected end of file [1]!");
        }
    }

    CodedInputStream::Limit lim = _coded_in->PushLimit(size);
    A4Message msg = _current_class_pool->read(class_id, _coded_in.get());
    _coded_in->PopLimit(lim);

    if (!msg) throw a4::Fatal("a4::io:InputStreamImpl - Failure to parse object!");
    return msg;
}

bool InputStreamImpl::handle_compressed_section(A4Message & msg) {
    if (msg.is<StartCompressedSection>()) {
        if (!start_compression(*msg.as<StartCompressedSection>())) {
            throw a4::Fatal("Unable to start compressed section!");
        }
        return true;
    } else if (msg.is<EndCompressedSection>()) {
        if(!stop_compression(*msg.as<EndCompressedSection>())) {
            throw a4::Fatal("Unable to stop compressed section!");
        }
        return true;
    }
    return false;
}

bool InputStreamImpl::handle_stream_command(A4Message & msg) {
    if (msg.class_id() < 100) return false;
    if (msg.class_id() > 200) return false;
    if (msg.is<StreamFooter>()) {
        // TODO: Process footer
        shared<StreamFooter> foot = msg.as<StreamFooter>();
        uint32_t size;
        if (!_coded_in->ReadLittleEndian32(&size)) {
            throw a4::Fatal("a4::io:InputStreamImpl - Unexpected end of file [3]!");
        }
        string magic;
        if (!_coded_in->ReadString(&magic, 8)) {
            throw a4::Fatal("a4::io:InputStreamImpl - Unexpected end of file [4]!");
        }
        if (0 != magic.compare(END_MAGIC)) {
            throw a4::Fatal("a4::io:InputStreamImpl - Corrupt footer! Read: ", magic);
        }
        if (_coded_in->ExpectAtEnd()) {
            _good = false; // Regular end of stream
            return true;
        }
        _current_header_index++;
        if (!read_header()) {
            if (_error) {
                throw a4::Fatal("a4::io:InputStreamImpl - Corrupt header!");
            }
            _good = false; // Slightly strange but regular end of stream
            return true;
        }
        _current_metadata = A4Message();
        if (!_current_metadata_refers_forward) {
            _current_metadata_index = 0;
            if (_metadata_per_header[_current_header_index].size() > 0)
                _current_metadata = _metadata_per_header[_current_header_index][0];
        } else {
            _current_metadata_index = -1;
        }
        return true;
    } else if (msg.is<StreamHeader>()) {
        throw a4::Fatal("a4::io:InputStreamImpl - Unexpected header!");
    } else if (msg.is<ProtoClass>()) {
        _current_class_pool->add_protoclass(*msg.as<ProtoClass>());
        return true;
    }
    return false;
}

bool InputStreamImpl::handle_metadata(A4Message & msg) {
    //if (_current_header_index > 0) {
    //    for (int i = 0; i < _current_header_index; i++) {
    //        _metadata_per_header[i].clear();
    //    }
    //}
    if (msg.metadata()) {
        _current_metadata_index++;
        if (_current_metadata_refers_forward) {
            _current_metadata = msg;
        } else {
            _current_metadata = A4Message();
            if (_metadata_per_header[_current_header_index].size() > _current_metadata_index)
                _current_metadata = _metadata_per_header[_current_header_index][_current_metadata_index];
        }
        _new_metadata = true;
        return true;
    }
    return false;
}

A4Message InputStreamImpl::next_message() {
    A4Message msg = bare_message();
    if(handle_compressed_section(msg)) return next_message();
    return msg;
}

A4Message InputStreamImpl::next(bool skip_metadata) {
    A4Message msg = next_message();
    if (handle_stream_command(msg)) return next(skip_metadata);
    if (handle_metadata(msg) && skip_metadata) return next(skip_metadata);
    return msg;
}

A4Message InputStreamImpl::next_bare_message() {
    A4Message msg = next_message();
    if (handle_stream_command(msg)) return msg;
    if (handle_metadata(msg)) return msg;
    return msg;
}

};}; // namespace a4::io
