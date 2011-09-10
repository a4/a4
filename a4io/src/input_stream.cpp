#define _FILE_OFFSET_BITS 64

#include <math.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/input_stream.h"
#include "gzip_stream.h"

using std::string;
using std::cerr;
using std::endl;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::FileInputStream;
using google::protobuf::io::CodedInputStream;
using namespace a4::io;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const int START_MAGIC_len = 8;
const int END_MAGIC_len = 8;
const uint32_t HIGH_BIT = 1<<31;


A4InputStream::A4InputStream(const string &input_file) {
    _fileno = open(input_file.c_str(), O_RDONLY);
    _file_in.reset(new FileInputStream(_fileno));
    _raw_in = _file_in;
    _inputname = input_file;
    startup();
}

A4InputStream::A4InputStream(shared<ZeroCopyInputStream> in, std::string name) {
    _fileno = 0;
    _file_in.reset();
    _raw_in = in;
    _inputname = name;
    startup();
}

void A4InputStream::startup() {
    // Initialize to defined state
    _compressed_in = NULL;
    _coded_in = NULL;
    _is_good = false;
    _new_metadata = false;
    _discovery_complete = false;
    _items_read = 0;
    _content_class_id = 0;
    _metadata_class_id = 0;
    _content_func = NULL;
    _current_metadata.reset();
    _current_metadata_refers_forward = false;
    _current_header_index = 0;
    _current_metadata_index = 0;


    _coded_in = new CodedInputStream(_raw_in.get());

    // Push limit of read bytes
    _coded_in->SetTotalBytesLimit(pow(1024,3), 900*pow(1024,2));

    int rh = read_header();
    if (_file_in && _file_in->GetErrno()) {
        std::cerr << "ERROR - a4::io:A4InputStream - Could not open '"<< _inputname \
                  << "' - error " << _file_in->GetErrno() << std::endl;
        throw _file_in->GetErrno();
    }
    if (rh == -2) {
        std::cerr << "ERROR - a4::io:A4InputStream - File Empty!" << std::endl; 
    } else if (rh == -1) {
        std::cerr << "ERROR - a4::io:A4InputStream - Header corrupted!" << std::endl; 
    } else _is_good = true;
    _current_header_index = 0;
}

A4InputStream::~A4InputStream() {
    delete _coded_in;
    if (_compressed_in) delete _compressed_in;
    _raw_in.reset();
    _file_in.reset();
}

int A4InputStream::read_header()
{
    string magic;
    if (!_coded_in->ReadString(&magic, 8))
        return -2;

    if (0 != magic.compare(START_MAGIC))
        return -1;

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size))
        return -1;

    uint32_t message_type = 0;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type))
            return -1;
    }
    if (!message_type == A4StreamHeader::kCLASSIDFieldNumber)
        return -1;

    A4StreamHeader h;
    CodedInputStream::Limit lim = _coded_in->PushLimit(size);
    if (!h.ParseFromCodedStream(_coded_in))
        return -1;
    _coded_in->PopLimit(lim);

    if (h.a4_version() != 1) {
        std::cerr << "ERROR - a4::io:A4InputStream - Unknown A4 stream version (";
        std::cerr << h.a4_version() << ")" << std::endl;
        return -1;
    }

    // check the main content type from the header
    if (h.has_content_class_id()) {
        _content_class_id = h.content_class_id();
        _content_func = internal::all_class_ids(_content_class_id);
        if (!_content_func) {
            std::cerr << "ERROR - a4::io:A4InputStream - Content Class " << _content_class_id << " unknown!" << std::endl;
            assert(_content_func);
        }

    } else {
        _content_class_id = 0;
    };
    if (h.has_metadata_class_id()) {
        _metadata_class_id = h.metadata_class_id();
        if (!internal::all_class_ids(_metadata_class_id)) {
            std::cerr << "ERROR - a4::io:A4InputStream - metadata Class " << _metadata_class_id << " unknown!" << std::endl;
        }
    } else {
        _metadata_class_id = 0;
    };

    _current_metadata_refers_forward = h.metadata_refers_forward();
    if (!_current_metadata_refers_forward && !_discovery_complete) {
        if (!_file_in) {
            std::cerr << "ERROR - a4::io:A4InputStream - Cannot read reverse metadata from non-seekable stream!" << std::endl;
            _is_good = false;
        } else if (!discover_all_metadata()) {
            std::cerr << "ERROR - a4::io:A4InputStream - Failed to discover metadata - file corrupted?" << std::endl;
            _is_good = false;
        }
        _current_metadata_index = 0;
        if (_metadata_per_header[_current_header_index].size() > 0) {
            _current_metadata = _metadata_per_header[_current_header_index][0];
            _new_metadata = true;
        }
    }

    return 0;
}

bool A4InputStream::discover_all_metadata() {
    assert(_metadata_per_header.size() == 0);

    int64_t size = 0;
    std::deque<uint64_t> headers;

    while (true) {
        int mark = 0;
        if (seek(-size - END_MAGIC_len, SEEK_END) == -1) return false;
        string magic;
        if (!_coded_in->ReadString(&magic, 8)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file during scan!" << std::endl; 
            return false;
        }
        if (seek(-size - END_MAGIC_len - 4, SEEK_END) == -1) return false;

        uint32_t footer_size = 0;
        if (!_coded_in->ReadLittleEndian32(&footer_size)) return false;
        uint32_t footer_msgsize  = END_MAGIC_len + 4 + footer_size + 8;
        uint64_t footer_start = - size - footer_msgsize;
        uint64_t footer_abs_start = seek(footer_start, SEEK_END);
        if (footer_abs_start == -1) return false;
        ReadResult rr = next(true);
        if (rr.class_id != A4StreamFooter::kCLASSIDFieldNumber) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unknown footer class_id " << rr.class_id << std::endl;
            return false;
        }
        auto footer = static_shared_cast<A4StreamFooter>(rr.object);
        size += footer->size() + footer_msgsize;

        std::vector<shared<Message>> _this_headers_metadata;
        auto offsets = footer->metadata_offsets();
        for (auto offset = offsets.begin(), end = offsets.end(); offset != end; offset++) {
            uint64_t metadata_start = footer_abs_start - footer->size() + *offset;
            if (seek(metadata_start, SEEK_SET) == -1) return false;
            ReadResult rr = next(true);
            if (rr.class_id != _metadata_class_id) {
                std::cerr << "ERROR - a4::io:A4InputStream - class_id is not metadata class_id: "
                          << rr.class_id << " != " << _metadata_class_id << std::endl;
                return false;
            }
            _this_headers_metadata.push_back(rr.object);
        }
        _metadata_per_header.push_front(_this_headers_metadata);

        uint64_t tell = seek(-size, SEEK_END);
        headers.push_front(tell);
        if (tell == -1) return false;
        if (tell == 0) break;
    }
    seek(headers[_current_header_index] + START_MAGIC_len, SEEK_SET);
    next(true);
    _discovery_complete = true;
    return true;
}

int64_t A4InputStream::seek(int64_t position, int whence) {
    assert(_fileno != 0);
    assert(!_compressed_in);
    delete _coded_in;
    _file_in.reset();
    _raw_in.reset();
    int64_t pos = lseek(_fileno, position, whence);
    if (pos == (off_t)-1) return -1;
    _file_in.reset(new FileInputStream(_fileno));
    _raw_in = _file_in;
    _coded_in = new CodedInputStream(_raw_in.get());
    _coded_in->SetTotalBytesLimit(pow(1024,3), 900*pow(1024,2));
    return pos;
};

bool A4InputStream::start_compression(const A4StartCompressedSection& cs) {
    assert(!_compressed_in);
    delete _coded_in;

    if (cs.compression() == A4StartCompressedSection_Compression_ZLIB) {
        _compressed_in = new GzipInputStream(_raw_in.get(), GzipInputStream::ZLIB);
    } else if (cs.compression() == A4StartCompressedSection_Compression_GZIP) {
        _compressed_in = new GzipInputStream(_raw_in.get(), GzipInputStream::GZIP);
    } else {
        std::cerr << "ERROR - a4::io:A4InputStream - Unknown compression type " << cs.compression() << std::endl;
        return false;
    }

    _coded_in = new CodedInputStream(_compressed_in);
    _coded_in->SetTotalBytesLimit(pow(1024,3), 900*pow(1024,2));
    return true;
}

bool A4InputStream::stop_compression(const A4EndCompressedSection& cs) {
    assert(_compressed_in);
    delete _coded_in;
    if (!_compressed_in->ExpectAtEnd()) {
        std::cerr << "ERROR - a4::io:A4InputStream - Compressed section did not end where it should!" << std::endl;
        _is_good = false;
        return false;
    }
    delete _compressed_in;
    _compressed_in = NULL;
    _coded_in = new CodedInputStream(_raw_in.get());
    return true;
}

void A4InputStream::reset_coded_stream() {
    delete _coded_in;
    if (_compressed_in) 
        _coded_in = new CodedInputStream(_compressed_in);
    else
        _coded_in = new CodedInputStream(_raw_in.get());
}

ReadResult A4InputStream::next(bool internal) {

    static int i = 0;
    if (i++ % 1000 == 0) reset_coded_stream();

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size)) {
        if (_compressed_in && _compressed_in->ByteCount() == 0) {
            std::cerr << "ERROR - a4::io:A4InputStream - Reading from compressed section failed!" << std::endl; 
        } else {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file or corruption [0]!" << std::endl; 
        }
        _is_good = false;
        return READ_ERROR;
    }

    uint32_t message_type = _content_class_id;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file [1]!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
    } else if (message_type == 0) {
        std::cerr << "ERROR - a4::io:A4InputStream - Default Object found while no default type is set!" << std::endl; 
        _is_good = false;
        return READ_ERROR;
    }

    string magic;
    int rh = 0;

    if (message_type == _content_class_id) {
        CodedInputStream::Limit lim = _coded_in->PushLimit(size);
        shared<Message> item = _content_func(_coded_in);
        _coded_in->PopLimit(lim);
        if (!item) {
            std::cerr << "ERROR - a4::io:A4InputStream - Failure to parse Item!" << std::endl;
            _is_good = false;
            return READ_ERROR;
        }
        ++_items_read;
        return ReadResult(message_type, item);
    }

    internal::from_stream_func from_stream = internal::all_class_ids(message_type);
    if (!from_stream) {
        std::cerr << "WARNING - a4::io:A4InputStream - Unknown message type: " << message_type << std::endl;
        if (!_coded_in->Skip(size)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Read error while skipping unknown message!"  << std::endl;
            _is_good = false;
            return READ_ERROR;
        }
        return next();
    }
    CodedInputStream::Limit lim = _coded_in->PushLimit(size);
    shared<Message> item = from_stream(_coded_in);
    _coded_in->PopLimit(lim);
    if (!item) {
        std::cerr << "ERROR - a4::io:A4InputStream - Failure to parse object!" << std::endl;
        _is_good = false;
        return READ_ERROR;
    }
    
    if (internal) return ReadResult(message_type, item);
        
    if (message_type == A4StreamFooter::kCLASSIDFieldNumber) {
        // TODO: Process footer
        auto foot = static_shared_cast<A4StreamFooter>(item);

        if (!_coded_in->ReadLittleEndian32(&size)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file [3]!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
        if (!_coded_in->ReadString(&magic, 8)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file [4]!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
        if (0 != magic.compare(END_MAGIC)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Corrupt footer! Read: "<< magic << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
        if (_coded_in->ExpectAtEnd()) {
            _is_good = false;
            return END_OF_STREAM;
        }
        _current_header_index++;
        rh = read_header();
        if (rh == -2) {
            _is_good = false;
            return END_OF_STREAM;
        } else if (rh == -1) {
            std::cerr << "ERROR - a4::io:A4InputStream - Corrupt header!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }

        _current_metadata.reset();
        if (!_current_metadata_refers_forward) {
            _current_metadata_index = 0;
            if (_metadata_per_header[_current_header_index].size() > 0)
                _current_metadata = _metadata_per_header[_current_header_index][0];
        }
        return next();
    } else if (message_type == A4StreamHeader::kCLASSIDFieldNumber) {
        // should not happen???
        std::cerr << "ERROR - a4::io:A4InputStream - Unexpected header!" << std::endl; 
        _is_good = false;
        return READ_ERROR;
        //return read_item(item);
    } else if (message_type == A4StartCompressedSection::kCLASSIDFieldNumber) {
        if (!start_compression(*static_cast<A4StartCompressedSection*>(item.get()))) {
            _is_good = false;
            return READ_ERROR;
        }
        return next();
    } else if (message_type == A4EndCompressedSection::kCLASSIDFieldNumber) {
        if (!stop_compression(*static_cast<A4EndCompressedSection*>(item.get()))) {
            _is_good = false;
            return READ_ERROR;
        }
        return next();
    } else if (message_type == _metadata_class_id) {
        if (_current_metadata_refers_forward) {
            _current_metadata = item;
        } else {
            _current_metadata_index++;
            _current_metadata.reset();
            if (_metadata_per_header[_current_header_index].size() > _current_metadata_index)
                _current_metadata = _metadata_per_header[_current_header_index][_current_metadata_index];
        }
        _new_metadata = true;
        return next();
    }
    return ReadResult(message_type, item);
}

