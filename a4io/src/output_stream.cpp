#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <errno.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "a4/output_stream.h"
#ifdef HAVE_SNAPPY
#include "snappy_stream.h"
#endif
#include "a4/proto/io/A4Stream.pb.h"

#include "gzip_stream.h"

using std::string;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::FileDescriptor;
using namespace a4::io;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const uint32_t HIGH_BIT = 1<<31;


A4OutputStream::A4OutputStream(const string &output_file, 
                               const string description, 
                               uint32_t content_class_id, 
                               uint32_t metadata_class_id) : 
    _output_name(output_file),
    _description(description),
    _fileno(-1),
    _compression(true),
    _opened(false),
    _closed(false),
    _metadata_refers_forward(false),
    _content_class_id(content_class_id),
    _metadata_class_id(metadata_class_id),
    _written_file_descriptors(new ::google::protobuf::SimpleDescriptorDatabase())
{

}

A4OutputStream::A4OutputStream(shared<google::protobuf::io::ZeroCopyOutputStream> out,
                           const std::string outname,
                           const std::string description, 
                           uint32_t content_class_id,
                           uint32_t metadata_class_id) : 
    _output_name(outname),
    _description(description),
    _fileno(0),
    _compression(true),
    _opened(false),
    _closed(false),
    _metadata_refers_forward(false),
    _content_class_id(content_class_id),
    _written_file_descriptors()
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
    _compressed_out.reset();
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

    _coded_out.reset(new CodedOutputStream(_raw_out.get()));
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
    _coded_out.reset();
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
    _coded_out.reset();
    uint64_t size = _raw_out->ByteCount();
    _coded_out.reset(new CodedOutputStream(_raw_out.get()));
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
    _coded_out.reset();
    if (_compressed_out) 
        _coded_out.reset(new CodedOutputStream(_compressed_out.get()));
    else
        _coded_out.reset(new CodedOutputStream(_raw_out.get()));
}

bool A4OutputStream::start_compression() {
    if (_compressed_out) return false;
    A4StartCompressedSection cs_header;
#ifdef HAVE_SNAPPY
    cs_header.set_compression(A4StartCompressedSection_Compression_SNAPPY);
#else
    cs_header.set_compression(A4StartCompressedSection_Compression_ZLIB);
#endif
    if (!write(A4StartCompressedSection::kCLASSIDFieldNumber, cs_header))
        throw a4::Fatal("Failed to start compression");
    _coded_out.reset();
#ifdef HAVE_SNAPPY
    _compressed_out.reset(new SnappyOutputStream(_raw_out.get()));
#else
    a4::io::GzipOutputStream::Options o;
    o.format = a4::io::GzipOutputStream::ZLIB;
    o.compression_level = 9;
    _compressed_out.reset(new a4::io::GzipOutputStream(_raw_out.get(), o));
#endif
    _coded_out.reset(new CodedOutputStream(_compressed_out.get()));
    return true;
};

bool A4OutputStream::stop_compression() {
    if (!_compressed_out) return false;
    A4EndCompressedSection cs_footer;
    if (!write(A4EndCompressedSection::kCLASSIDFieldNumber, cs_footer))
        throw a4::Fatal("Failed to stop compression");
    _coded_out.reset();
    _compressed_out->Flush();
    _compressed_out->Close();
    _compressed_out.reset();
    _coded_out.reset(new CodedOutputStream(_raw_out.get()));
    return true;
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
    for (uint32_t i = 0; i < metadata_positions.size(); i++)
        footer.add_metadata_offsets(metadata_positions[i]);
    if (_content_class_id != 0)
        footer.set_content_count(_content_count);
    write(A4StreamFooter::kCLASSIDFieldNumber, footer);
    _coded_out->WriteLittleEndian32(footer.ByteSize());
    _coded_out->WriteString(END_MAGIC);
    return true;
}

void get_descriptors_recursively(
    std::vector<const google::protobuf::FileDescriptor*>& file_descriptors, 
    const google::protobuf::FileDescriptor* file_descriptor) 
{
    foreach (const google::protobuf::FileDescriptor* fd, file_descriptors) {
        if (fd->name() == file_descriptor->name())
            return; // We have seen this one before
    }
        
    for (int i = 0; i < file_descriptor->dependency_count(); i++)
        get_descriptors_recursively(file_descriptors, file_descriptor->dependency(i));
    
    file_descriptors.push_back(file_descriptor);
}

void A4OutputStream::write_a4proto(const google::protobuf::Message &msg)
{
    std::vector<const google::protobuf::FileDescriptor*> file_descriptors;
    
    get_descriptors_recursively(file_descriptors, msg.GetDescriptor()->file());
    
    foreach (const google::protobuf::FileDescriptor* fd, file_descriptors) {
        google::protobuf::FileDescriptorProto fdp;
        fd->CopyTo(&fdp); // Necessary to have it in proto form
        {
            google::protobuf::LogSilencer silencer;
            if (!_written_file_descriptors->Add(fdp))
                continue; // This filedescriptor has been written as a a4proto before.
        }
        
        A4Proto a4proto;
        fd->CopyTo(a4proto.mutable_file_descriptor());
        write(A4Proto::kCLASSIDFieldNumber, a4proto);
        
        for (int i = 0; i < fd->message_type_count(); i++) {
            const google::protobuf::Descriptor* d = fd->message_type(i);
            const google::protobuf::FieldDescriptor* fdesc = d->FindFieldByName("CLASS_ID");
            if (!fdesc) continue; // Doesn't have a CLASS_ID, ignore
            set_written_classid(fdesc->number());
        }
    }
}

bool A4OutputStream::write(uint32_t class_id, const google::protobuf::Message &msg)
{
    // If it is a custom message class, write it to the file
    if (class_id >= uint32_t(a4::io::FIRST_CUSTOM_MESSAGE_CLASS) && not have_written_classid(class_id))
        write_a4proto(msg);
        
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
    msg.SerializeWithCachedSizes(_coded_out.get());
    return true;
}


