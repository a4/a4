#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/writer.h"
#include "a4/proto/io/A4Stream.pb.h"

using std::string;
using namespace google::protobuf::io;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const uint32_t HIGH_BIT = 1<<31;


Writer::Writer(const string &output_file, const string description, uint32_t content_class_id, uint32_t metadata_class_id, bool compression) :
    _raw_out(0),
    _compressed_out(0),
    _coded_out(0),
    _compression(compression),
    _content_count(0),
    _bytes_written(0),
    _content_class_id(content_class_id),
    _metadata_class_id(metadata_class_id)
{
    _raw_out = new FileOutputStream(open(output_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT | O_DIRECT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    _coded_out = new CodedOutputStream(_raw_out);
    write_header(description);
    if (compression) start_compression();
}

Writer::~Writer()
{
    if (_compressed_out) stop_compression();
    write_footer();
    delete _coded_out;
    _raw_out->Close(); // return false on error
    delete _raw_out;
}

uint64_t Writer::get_bytes_written() {
    assert(!_compressed_out);
    delete _coded_out;
    uint64_t size = _raw_out->ByteCount();
    _coded_out = new CodedOutputStream(_raw_out);
    return size;
}

bool Writer::write(Streamable &obj)
{
    MessagePtr msg = obj.get_message();
    if (!msg.get()) return false;

    uint32_t class_id = obj.get_class_id(); //msg->GetDescriptor()->FindFieldByName("CLASS_ID")->number();
    assert(class_id != 0);
    return write(class_id, *msg);
}

bool Writer::metadata(MetaData &msg)
{
    // TODO: Add back-reference to metadata from footer
    return write(msg);
}

bool Writer::start_compression() {
    if (_compressed_out) return false;
    a4::io::A4StartCompressedSection cs_header;
    cs_header.set_compression(a4::io::A4StartCompressedSection_Compression_ZLIB);
    write(a4::io::A4StartCompressedSection::kCLASSIDFieldNumber, cs_header);
    _bytes_written += _coded_out->ByteCount();
    delete _coded_out;
    //_raw_out->Flush();

    GzipOutputStream::Options o;
    o.format = GzipOutputStream::ZLIB;
    o.compression_level = 9;
    _compressed_out = new GzipOutputStream(_raw_out, o);
    _coded_out = new CodedOutputStream(_compressed_out);
};

bool Writer::stop_compression() {
    if (!_compressed_out) return false;
    a4::io::A4EndCompressedSection cs_footer;
    write(a4::io::A4EndCompressedSection::kCLASSIDFieldNumber, cs_footer);
    delete _coded_out;
    _compressed_out->Flush();
    _compressed_out->Close();
    _bytes_written += _compressed_out->ByteCount();
    delete _compressed_out;
    _compressed_out = NULL;
    _raw_out->Flush();
    _coded_out = new CodedOutputStream(_raw_out);
};

bool Writer::write_header(string description) {
    _coded_out->WriteString(START_MAGIC);
    a4::io::A4StreamHeader header;
    header.set_a4_version(1);
    header.set_description(description);
    if (_content_class_id != 0)
        header.set_content_class_id(_content_class_id);
    if (_metadata_class_id != 0)
        header.set_metadata_class_id(_metadata_class_id);

    return write(a4::io::A4StreamHeader::kCLASSIDFieldNumber, header);
}

bool Writer::write_footer() {
    assert(!_compressed_out);
    a4::io::A4StreamFooter footer;
    footer.set_size(get_bytes_written());
    if (_content_class_id != 0)
        footer.set_content_count(_content_count);
    write(a4::io::A4StreamFooter::kCLASSIDFieldNumber, footer);
    _coded_out->WriteLittleEndian32(footer.ByteSize());
    _coded_out->WriteString(END_MAGIC);
    return true;
}

bool Writer::write(uint32_t class_id, ::google::protobuf::Message &msg)
{
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


