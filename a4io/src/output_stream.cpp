#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <errno.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/output_stream.h"
#include "snappy_stream.h"
#include "a4/proto/io/A4Stream.pb.h"

using std::string;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::io::GzipOutputStream;
using namespace a4::io;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const uint32_t HIGH_BIT = 1<<31;


A4OutputStream::A4OutputStream(const string &output_file, 
                               const string description, 
                               uint32_t content_class_id, 
                               uint32_t metadata_class_id) : 
    _content_class_id(content_class_id),
    _metadata_class_id(metadata_class_id),
    _metadata_refers_forward(false),
    _compression(true),
    _output_name(output_file),
    _description(description),
    _opened(false),
    _closed(false)
{
    _fileno = -1;
}

A4OutputStream::A4OutputStream(shared<google::protobuf::io::ZeroCopyOutputStream> out,
                           const std::string outname,
                           const std::string description, 
                           uint32_t content_class_id,
                           uint32_t metadata_class_id) : 
    _content_class_id(content_class_id),
    _metadata_class_id(metadata_class_id),
    _metadata_refers_forward(false),
    _compression(true),
    _output_name(outname),
    _description(description),
    _opened(false),
    _closed(false)
{
    _raw_out = out;
}

A4OutputStream::~A4OutputStream()
{
    if (!_closed && _opened) close();
}

bool A4OutputStream::open() {
    if (_opened) return false;
    _opened = true;
    _compressed_out = NULL;
    _content_count = 0;

    if (_fileno == -1) {
        int fd = ::open(_output_name.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0) {
            std::cerr << "ERROR - A4IO:A4OutputStream - Could not open '" << _output_name \
                      << "' - error: " << strerror(errno) << std::endl;
            return false;
        }
        _file_out.reset(new FileOutputStream(fd));
        _raw_out = _file_out;
    } else {
        _file_out.reset();
    }

    _coded_out = new CodedOutputStream(_raw_out.get());
    write_header(_description);
    if (_compression) start_compression();
    return true;
}

bool A4OutputStream::close() {
    assert(!_closed);
    assert(_opened);
    _closed = true;
    if (_compressed_out) stop_compression();
    write_footer();
    delete _coded_out;
    if (_file_out && !_file_out->Close()) {
        std::cerr << "ERROR - A4IO:A4OutputStream - Error on closing: " << _file_out->GetErrno() << std::endl;
        return false;
    }; // return false on error
    _file_out.reset();
    _raw_out.reset();
    return true;
}


uint64_t A4OutputStream::get_bytes_written() {
    assert(!_compressed_out);
    delete _coded_out;
    uint64_t size = _raw_out->ByteCount();
    _coded_out = new CodedOutputStream(_raw_out.get());
    return size;
}

bool A4OutputStream::write(const google::protobuf::Message &msg)
{
    if (!_opened) if(!open()) { return false; };
    uint32_t class_id = msg.GetDescriptor()->FindFieldByName("CLASS_ID")->number();
    if (class_id == _metadata_class_id) {
        if (_compressed_out) {
            stop_compression();
            metadata_positions.push_back(get_bytes_written());
            bool res = write(class_id, msg);
            start_compression();
            return res;
        } else {
            metadata_positions.push_back(get_bytes_written());
            return write(class_id, msg);
        }
    } else return write(class_id, msg);
}

void A4OutputStream::reset_coded_stream() {
    delete _coded_out;
    if (_compressed_out) 
        _coded_out = new CodedOutputStream(_compressed_out);
    else
        _coded_out = new CodedOutputStream(_raw_out.get());
}

bool A4OutputStream::start_compression() {
    if (_compressed_out) return false;
    A4StartCompressedSection cs_header;
    cs_header.set_compression(A4StartCompressedSection_Compression_SNAPPY);
    write(A4StartCompressedSection::kCLASSIDFieldNumber, cs_header);
    delete _coded_out;
    //GzipOutputStream::Options o;
    //o.format = GzipOutputStream::ZLIB;
    //o.compression_level = 9;
    //_compressed_out = new GzipOutputStream(_raw_out.get(), o);
    _compressed_out = new SnappyOutputStream(_raw_out.get());
    _coded_out = new CodedOutputStream(_compressed_out);
};

bool A4OutputStream::stop_compression() {
    if (!_compressed_out) return false;
    A4EndCompressedSection cs_footer;
    write(A4EndCompressedSection::kCLASSIDFieldNumber, cs_footer);
    delete _coded_out;
    _compressed_out->Flush();
    _compressed_out->Close();
    delete _compressed_out;
    _compressed_out = NULL;
    _coded_out = new CodedOutputStream(_raw_out.get());
};

bool A4OutputStream::write_header(string description) {
    _coded_out->WriteString(START_MAGIC);
    A4StreamHeader header;
    header.set_a4_version(1);
    header.set_description(description);
    header.set_metadata_refers_forward(_metadata_refers_forward);
    if (_content_class_id != 0)
        header.set_content_class_id(_content_class_id);
    if (_metadata_class_id != 0)
        header.set_metadata_class_id(_metadata_class_id);

    return write(A4StreamHeader::kCLASSIDFieldNumber, header);
}

bool A4OutputStream::write_footer() {
    assert(!_compressed_out);
    A4StreamFooter footer;
    footer.set_size(get_bytes_written());
    footer.set_metadata_refers_forward(_metadata_refers_forward);
    for (int i = 0; i < metadata_positions.size(); i++)
        footer.add_metadata_offsets(metadata_positions[i]);
    if (_content_class_id != 0)
        footer.set_content_count(_content_count);
    write(A4StreamFooter::kCLASSIDFieldNumber, footer);
    _coded_out->WriteLittleEndian32(footer.ByteSize());
    _coded_out->WriteString(END_MAGIC);
    return true;
}

bool A4OutputStream::write(uint32_t class_id, const google::protobuf::Message &msg)
{
    if (_coded_out->ByteCount() > 100000000) reset_coded_stream();

    string message;
    if (!msg.SerializeToString(&message))
        return false;

    uint32_t size = message.size();
    if (class_id == _content_class_id) {
        _content_count++;
        _coded_out->WriteLittleEndian32(size);
    } else {
        _coded_out->WriteLittleEndian32(size | HIGH_BIT );
        _coded_out->WriteLittleEndian32(class_id);
    }
    msg.SerializeWithCachedSizes(_coded_out);
    return true;
}


