/**
 * ProtoBuf Writer
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#include <string>

#include <boost/shared_ptr.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "PBWriter.h"

using namespace std;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const uint32_t HIGH_BIT = (1<<31);

Writer::Writer(const fs::path &output_file, string content_name = "", uint32_t content_type = 0):
    _output(output_file.string().c_str(),
            ios::out | ios::trunc | ios::binary),
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
    return write(MessagePtr(&header));
}

bool Writer::write_footer() {
    A4StreamFooter footer;
    footer.set_size(_bytes_written);
    if (_content_type != 0)
        footer.set_content_count(_content_count);
    write(MessagePtr(&footer));
    _coded_out->WriteLittleEndian32(footer.ByteSize());
    _coded_out->WriteString(END_MAGIC);
    _bytes_written += END_MAGIC.size();
    return true;
}

bool Writer::write(Writer::MessagePtr msg)
{
    string message;
    if (!msg->SerializeToString(&message))
        return false;

    uint32_t type = msg->GetDescriptor()->FindFieldByName("CLASS_ID")->number();
    uint32_t size = message.size();
    if (type == _content_type) _content_count++;
    if (type != _previous_type) {
        _coded_out->WriteLittleEndian32(size | HIGH_BIT );
        _coded_out->WriteLittleEndian32(type);
        _bytes_written += 8;
        _previous_type = type;
    } else {
        _coded_out->WriteLittleEndian32(size);
        _bytes_written += 4;
    }

    _coded_out->WriteString(message);
    _bytes_written += size;
    return true;
}
