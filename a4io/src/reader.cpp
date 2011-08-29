#include <math.h>
#include <iostream>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/reader.h"

const uint32_t HIGH_BIT = 1<<31;

using std::string;

Reader::Reader(const string &input_file):
    _input(input_file.c_str(), std::ios::in | std::ios::binary),
    _items_read(0)
{
    _last_meta_data.reset();
    _raw_in.reset(new ::google::protobuf::io::IstreamInputStream(&_input));
    _coded_in.reset(new ::google::protobuf::io::CodedInputStream(_raw_in.get()));

    // Push limit of read bytes
    _coded_in->SetTotalBytesLimit(pow(1024,3), 900*pow(1024,2));

    int rh = _read_header();
    if (rh == -2) {
        std::cerr << "ERROR - A4IO:Reader - File Empty!" << std::endl; 
    } else if (rh == -1) {
        std::cerr << "ERROR - A4IO:Reader - Header corrupted!" << std::endl; 
    } else _is_good = true;
}

int Reader::_read_header()
{
    //uint64_t header_position = _coded_in->tell();

    string magic;
    if (!_coded_in->ReadString(&magic, 8))
        return -2;

    if (0 != magic.compare("A4STREAM"))
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
    if (!message_type == a4::io::A4StreamHeader::kCLASSIDFieldNumber)
        return -1;

    string message;
    if (!_coded_in->ReadString(&message, size))
        return -1;

    a4::io::A4StreamHeader h;
    if (!h.ParseFromString(message))
        return -1;

    if (h.a4_version() != 1) {
        std::cerr << "ERROR - A4IO:Reader - Unknown A4 stream version (";
        std::cerr << h.a4_version() << ")" << std::endl;
        return -1;
    }

    // check the main content type from the header
    if (h.has_content_class_id()) {
        _content_class_id = h.content_class_id();
        _content_func = all_class_ids[_content_class_id];
        if (!_content_func) {
            std::cerr << "ERROR - A4IO:Reader - Content Class " << _content_class_id << " unknown!" << std::endl;
            assert(_content_func);
        }

    } else {
        _content_class_id = 0;
    };

    return 0;
}

ReadResult Reader::read() {
    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size)) {
        std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [0]!" << std::endl; 
        _is_good = false;
        return READ_ERROR;
    }

    uint32_t message_type = _content_class_id;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type)) {
            std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [1]!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
    } else if (message_type == 0) {
        std::cerr << "ERROR - A4IO:Reader - Default Object found while no default type is set!" << std::endl; 
        _is_good = false;
        return READ_ERROR;

    }

    string message;
    if (!_coded_in->ReadString(&message, size)) {
        std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [2]!" << std::endl; 
        _is_good = false;
        return READ_ERROR;
    }

    string magic;
    int rh = 0;
    
    if (message_type == _content_class_id) {
        boost::shared_ptr<Streamable> item = _content_func(message);
        if (!item) {
            std::cerr << "ERROR - A4IO:Reader - Failure to parse Item!" << std::endl;
            _is_good = false;
            return READ_ERROR;
        }
        ++_items_read;
        return ReadResult(message_type, item);
    } else if (message_type == a4::io::A4StreamFooter::kCLASSIDFieldNumber) {
        // TODO: Process footer
        if (!_coded_in->ReadLittleEndian32(&size)) {
            std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [3]!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
        if (!_coded_in->ReadString(&magic, 8)) {
            std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [4]!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
        if (0 != magic.compare("KTHXBYE4")) {
            std::cerr << "ERROR - A4IO:Reader - Corrupt footer!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
        if (_coded_in->ExpectAtEnd()) {
            _is_good = false;
            return END_OF_STREAM;
        }
        rh = _read_header();
        if (rh == -2) {
            _is_good = false;
            return END_OF_STREAM;
        } else if (rh == -1) {
            std::cerr << "ERROR - A4IO:Reader - Corrupt header!" << std::endl; 
            _is_good = false;
            return READ_ERROR;
        }
        return read();
    } else if (message_type == a4::io::A4StreamHeader::kCLASSIDFieldNumber) {
        // should not happen???
        std::cerr << "ERROR - A4IO:Reader - Unexpected header!" << std::endl; 
        _is_good = false;
        return READ_ERROR;
        //return read_item(item);
    } else if (all_class_ids[message_type]) {
        boost::shared_ptr<Streamable> item = all_class_ids[message_type](message);
        if (!item) {
            std::cerr << "ERROR - A4IO:Reader - Failure to parse object!" << std::endl;
            _is_good = false;
            return READ_ERROR;
        }
        if (message_type == _metadata_class_id) _last_meta_data = boost::dynamic_pointer_cast<MetaData>(item);
        return ReadResult(message_type, item);
    }
    std::cerr << "ERROR - A4IO:Reader - Unknown message type: " << message_type << std::endl; 
    return READ_ERROR;
}

