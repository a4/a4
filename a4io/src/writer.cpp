#include <string>

#include <boost/shared_ptr.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/writer.h"
#include "a4/proto/io/A4Stream.pb.h"

using std::string;


const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const uint32_t HIGH_BIT = 1<<31;


Writer::Writer(const string &output_file, const string description, uint32_t content_class_id, uint32_t metadata_class_id) :
    _output(output_file.c_str(), std::ios::out | std::ios::trunc | std::ios::binary),
    _content_count(0),
    _bytes_written(0),
    _content_class_id(content_class_id),
    _metadata_class_id(metadata_class_id)
{
    _raw_out.reset(new ::google::protobuf::io::OstreamOutputStream(&_output));
    _coded_out.reset(new ::google::protobuf::io::CodedOutputStream(_raw_out.get()));
    write_header(description);
}

Writer::~Writer()
{
    write_footer();
    _coded_out.reset();
    _raw_out.reset();
    _output.close();
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

bool Writer::write_header(string description) {
    _coded_out->WriteString(START_MAGIC);
    _bytes_written += START_MAGIC.size();

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

    a4::io::A4StreamFooter footer;
    footer.set_size(_bytes_written);
    if (_content_class_id != 0)
        footer.set_content_count(_content_count);
    write(a4::io::A4StreamFooter::kCLASSIDFieldNumber, footer);

    _coded_out->WriteLittleEndian32(footer.ByteSize());
    _coded_out->WriteString(END_MAGIC);
    _bytes_written += END_MAGIC.size();

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
        _bytes_written += 4;
    } else {
        _coded_out->WriteLittleEndian32(size | HIGH_BIT );
        _coded_out->WriteLittleEndian32(class_id);
        _bytes_written += 8;
    }

    _coded_out->WriteString(message);
    _bytes_written += size;
    return true;
}


