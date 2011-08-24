#ifndef PROTOBUF_READER_IMPL_H
#define PROTOBUF_READER_IMPL_H

#include "a4/reader.h"

#include <math.h>
#include <iostream>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/proto/io/A4Stream.pb.h"

using namespace a4::io;

#undef HIGH_BIT
#define HIGH_BIT (uint32_t(1<<31))

template <class Item, class MetaData>
Reader<Item, MetaData>::Reader(const string &input_file):
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
    }
}

template <class Item, class MetaData>
int Reader<Item, MetaData>::_read_header()
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
    if (!message_type == A4StreamHeader::kCLASSIDFieldNumber)
        return -1;

    string message;
    if (!_coded_in->ReadString(&message, size))
        return -1;

    A4StreamHeader h;
    if (!h.ParseFromString(message))
        return -1;

    // check the main content type from the header
    if (h.has_content_class_id()) {
        if (!Item::kCLASSIDFieldNumber == h.content_class_id()) {
            return -1;
        };
    };

    return 0;
}

template <class Item, class MetaData>
ReadResult Reader<Item, MetaData>::read(Item &item)
{
    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size)) {
        std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [0]!" << std::endl; 
        return FAIL;
    }

    uint32_t message_type = Item::kCLASSIDFieldNumber;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type)) {
            std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [1]!" << std::endl; 
            return FAIL;
        }
    }

    string message;
    if (!_coded_in->ReadString(&message, size)) {
        std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [2]!" << std::endl; 
        return FAIL;
    }

    string magic;
    int rh = 0;
    switch(message_type) {
        case Item::kCLASSIDFieldNumber:
            if (!item.ParseFromString(message)) {
                std::cerr << "ERROR - A4IO:Reader - Failure to parse Item!" << std::endl;
                return FAIL;
            }
            ++_items_read;
            return READ_ITEM;
        case MetaData::kCLASSIDFieldNumber:
            _last_meta_data.reset(new MetaData());
            if (!_last_meta_data->ParseFromString(message)) {
                std::cerr << "ERROR - A4IO:Reader - Failure to parse MetaData!" << std::endl; 
                return FAIL;
            }
            return NEW_METADATA;
        case A4StreamFooter::kCLASSIDFieldNumber:
            // TODO: Process footer
            if (!_coded_in->ReadLittleEndian32(&size)) {
                std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [3]!" << std::endl; 
                return FAIL;
            }
            if (!_coded_in->ReadString(&magic, 8)) {
                std::cerr << "ERROR - A4IO:Reader - Unexpected end of file [4]!" << std::endl; 
                return FAIL;
            }
            if (0 != magic.compare("KTHXBYE4")) {
                std::cerr << "ERROR - A4IO:Reader - Corrupt footer!" << std::endl; 
                return FAIL;
            }
            if (_coded_in->ExpectAtEnd())
                return STREAM_END;
            rh = _read_header();
            if (rh == -2) return STREAM_END;
            if (rh == -1) {
                std::cerr << "ERROR - A4IO:Reader - Corrupt header!" << std::endl; 
                return FAIL;
            }
            return read(item);
        case A4StreamHeader::kCLASSIDFieldNumber:
            // should not happen???
            std::cerr << "ERROR - A4IO:Reader - Unexpected header!" << std::endl; 
            return FAIL;
            //return read_item(item);
    }
    std::cerr << "ERROR - A4IO:Reader - Unknown message type: " << message_type << std::endl; 
    return FAIL;
}

#endif
