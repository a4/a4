/**
 * ProtoBuf Writer
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#include <string>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "PBWriter.h"

using namespace std;

using pb::Writer;

Writer::Writer(const fs::path &output_file):
    _output(output_file.string().c_str(),
            ios::out | ios::trunc | ios::binary),
    _events_written(0)
{

    _raw_out.reset(new ::google::protobuf::io::OstreamOutputStream(&_output));
    _raw_compressed_out.reset(new ::google::protobuf::io::GzipOutputStream(_raw_out.get()));
    _coded_out.reset(new ::google::protobuf::io::CodedOutputStream(_raw_compressed_out.get()));

    _coded_out->WriteLittleEndian32(_events_written);
}

Writer::~Writer()
{
    _coded_out.reset();
    _raw_out.reset();
    _raw_compressed_out.reset();

    _output.seekp(0);

    _raw_out.reset(new ::google::protobuf::io::OstreamOutputStream(&_output));
    _raw_compressed_out.reset(new ::google::protobuf::io::GzipOutputStream(_raw_out.get()));
    _coded_out.reset(new ::google::protobuf::io::CodedOutputStream(_raw_compressed_out.get()));
    _coded_out->WriteLittleEndian32(_events_written);

    _coded_out.reset();
    _raw_out.reset();
    _raw_compressed_out.reset();

    _output.close();
}

bool Writer::write(const Event &event)
{
    string message;
    if (!event.SerializeToString(&message))
        return false;

    _coded_out->WriteVarint32(message.size());
    _coded_out->WriteString(message);

    ++_events_written;

    return true;
}
