#ifndef PROTOBUF_WRITER_IMPL_H
#define PROTOBUF_WRITER_IMPL_H

#include <string>

#include <boost/shared_ptr.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/writer.h"

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";

#undef HIGH_BIT
#define HIGH_BIT uint32_t(1<<31)

Writer::Writer(const string &output_file, const string content_name = "", uint32_t content_type = 0):
    _output(output_file.c_str(),
            std::ios::out | std::ios::trunc | std::ios::binary),
    _content_count(0),
    _bytes_written(0),
    _content_type(content_type)
{
    _raw_out.reset(new ::google::protobuf::io::OstreamOutputStream(&_output));
    _coded_out.reset(new ::google::protobuf::io::CodedOutputStream(_raw_out.get()));
    write_header(content_name);
}

Writer::~Writer()
{
    write_footer();
    _coded_out.reset();
    _raw_out.reset();
    _output.close();
}

bool Writer::write_header(string content_name) {
    _coded_out->WriteString(START_MAGIC);
    _bytes_written += START_MAGIC.size();

    A4StreamHeader header;
    header.set_a4_version(1);
    if (_content_type != 0)
        header.set_content(content_name);

    return write(header);
}

bool Writer::write_footer() {

    A4StreamFooter footer;
    footer.set_size(_bytes_written);
    if (_content_type != 0)
        footer.set_content_count(_content_count);
    write(footer);

    _coded_out->WriteLittleEndian32(footer.ByteSize());
    _coded_out->WriteString(END_MAGIC);
    _bytes_written += END_MAGIC.size();

    return true;
}

bool Writer::write(Message &msg)
{
    string message;
    if (!msg.SerializeToString(&message))
        return false;

    uint32_t type = msg.GetDescriptor()->FindFieldByName("CLASS_ID")->number();
    uint32_t size = message.size();
    if (type == _content_type) {
        _content_count++;
        _coded_out->WriteLittleEndian32(size);
        _bytes_written += 4;
    } else {
        _coded_out->WriteLittleEndian32(size | HIGH_BIT );
        _coded_out->WriteLittleEndian32(type);
        _bytes_written += 8;
    }

    _coded_out->WriteString(message);
    _bytes_written += size;
    return true;
}

bool Writer::write_metadata(Message &msg)
{
    // TODO: Add back-reference to metadata from footer
    return write(msg);
}

#endif
