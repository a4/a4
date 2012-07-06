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

#include <a4/register.h>
#include <a4/message.h>
#include <a4/output_stream.h>
#include <a4/io/A4Stream.pb.h>

#include "gzip_stream.h"
#include "compressed_stream.h"

using std::string;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::FileDescriptor;
using namespace a4::io;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const uint32_t HIGH_BIT = 1<<31;


OutputStream::OutputStream(const string &output_file, 
                               const string description) : 
    _output_name(output_file),
    _description(description),
    _fileno(-1),
    _compression(true),
    _compression_level(9),
#ifdef HAVE_SNAPPY    
    _compression_type(SNAPPY),
#else
    _compression_type(ZLIB),
#endif    
    _opened(false),
    _closed(false),
    _metadata_refers_forward(false),
    _next_class_id(0),
    _next_metadata_class_id(1)
{
    _class_id_counts.resize(200);
}

OutputStream::OutputStream(shared<google::protobuf::io::ZeroCopyOutputStream> out,
                           const std::string outname,
                           const std::string description) : 
    _output_name(outname),
    _description(description),
    _fileno(0),
    _compression(true),
    _opened(false),
    _closed(false),
    _metadata_refers_forward(false)
{
    _raw_out = out;
    _class_id_counts.resize(200);
}

OutputStream::~OutputStream()
{
    if (!_closed && _opened) close();
}

bool OutputStream::open() {
    if (_opened) return false;
    _opened = true;
    _compressed_out.reset();

    if (_fileno == -1) {
        int fd = ::open(_output_name.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0) {
            std::cerr << "ERROR - A4IO:OutputStream - Could not open '" << _output_name \
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

bool OutputStream::close() {
    if(_closed) return true;
    assert(_opened);
    _closed = true;
    if (_compressed_out) stop_compression();
    write_footer();
    _coded_out.reset();
    if (_file_out && !_file_out->Close()) {
        std::cerr << "ERROR - A4IO:OutputStream - Error on closing: " << _file_out->GetErrno() << std::endl;
        return false;
    }; // return false on error
    _file_out.reset();
    _raw_out.reset();
    return true;
}

/// Returns the current number of bytes written.
/// Note: If compression is currently active, it is temporarily halted.
uint64_t OutputStream::get_bytes_written() {
    bool compressed = !!(_compressed_out);
    if (compressed) stop_compression();
    assert(_raw_out);
    _coded_out.reset();
    uint64_t size = _raw_out->ByteCount();
    _coded_out.reset(new CodedOutputStream(_raw_out.get()));
    if (compressed) start_compression();
    return size;
}

/// Get a class ID. Even for data, Odd for Metadata.
uint32_t OutputStream::find_class_id(const google::protobuf::Descriptor* d, bool metadata) {
    auto i = _class_id.find(d->full_name());
    if (i != _class_id.end()) {
        if (metadata != (i->second % 2 == 1)) {
            FATAL("Sorry, you can use the same Message class (", d->full_name() ,") either for metadata or for content, but not both.");
        }
        return i->second;
    }
    uint32_t class_id = 0;
    if (metadata) {
        class_id = _next_metadata_class_id;
        _next_metadata_class_id += 2;
    } else {
        class_id = _next_class_id;
        _next_class_id += 2;
    }
    if (_next_metadata_class_id == 101) _next_metadata_class_id += 100; // skip the 100's, since the a4 messages are there.
    if (_next_class_id == 100) _next_class_id += 100; // skip the 100's, since the a4 messages are there.
    write_protoclass(class_id, d);
    _class_id[d->full_name()] = class_id;
    assert((class_id % 2) == metadata);
    return class_id;
}

bool OutputStream::write(const google::protobuf::Message &msg)
{
    if (!_opened) if(!open()) { return false; };
    uint32_t class_id = find_class_id(msg.GetDescriptor(), false);
    return write(class_id, msg);
}

bool OutputStream::write(shared<const A4Message> msg)
{
    if (!_opened) if(!open()) { return false; };
    uint32_t class_id = find_class_id(msg->descriptor(), false);
    return write(class_id, msg);
}

bool OutputStream::metadata(const google::protobuf::Message &msg) {
    if (!_opened) if(!open()) { return false; };
    uint32_t class_id = find_class_id(msg.GetDescriptor(), true);
    metadata_positions.push_back(get_bytes_written());
    return write(class_id, msg);
}

void OutputStream::reset_coded_stream() {
    _coded_out.reset();
    if (_compressed_out) 
        _coded_out.reset(new CodedOutputStream(_compressed_out.get()));
    else
        _coded_out.reset(new CodedOutputStream(_raw_out.get()));
}

OutputStream& OutputStream::set_compression(CompressionType t, int level) {
    _compression = (t != UNCOMPRESSED);
    _compression_type = t;
    if (level < 1 || level > 9) a4::Fatal("Only compression levels between 1 and 9 are meaningful.");
    _compression_level = level;
    #ifndef HAVE_SNAPPY
    if (t == SNAPPY) a4::Fatal("Compression with 'snappy' is specified, but the library is not compiled in!");
    #endif
    return *this;
}

bool OutputStream::start_compression() {
    if (_compressed_out) return false;
    StartCompressedSection cs_header;
    
    if (_compression_type == SNAPPY) cs_header.set_compression(StartCompressedSection_Compression_SNAPPY);
    else if (_compression_type == ZLIB) cs_header.set_compression(StartCompressedSection_Compression_ZLIB);
    else if (_compression_type == LZ4) cs_header.set_compression(StartCompressedSection_Compression_LZ4);
    
    if (!write(_fixed_class_id<StartCompressedSection>(), cs_header))
        FATAL("Failed to start compression");
    _coded_out.reset();

    switch (_compression_type) {
    case SNAPPY:
    {
#ifdef HAVE_SNAPPY
        _compressed_out.reset(new SnappyOutputStream(_raw_out.get()));
#else
        FATAL("Snappy compression not compiled in!");
#endif
        break;
    }
    case LZ4:
    {
        _compressed_out.reset(new LZ4OutputStream(_raw_out.get()));
        break;
    }
    case ZLIB:
    {
        a4::io::GzipOutputStream::Options o;
        o.format = a4::io::GzipOutputStream::ZLIB;
        o.compression_level = _compression_level;
        _compressed_out.reset(new a4::io::GzipOutputStream(_raw_out.get(), o));
        break;
    }
    default:
        FATAL("Control should not reach here.");
    }
    _coded_out.reset(new CodedOutputStream(_compressed_out.get()));
    return true;
};

bool OutputStream::stop_compression() {
    if (!_compressed_out) return false;
    EndCompressedSection cs_footer;
    if (!write(_fixed_class_id<EndCompressedSection>(), cs_footer))
        FATAL("Failed to stop compression");
    _coded_out.reset();
    _compressed_out->Flush();
    _compressed_out->Close();
    _compressed_out.reset();
    _coded_out.reset(new CodedOutputStream(_raw_out.get()));
    return true;
};

bool OutputStream::write_header(string description) {
    _coded_out->WriteString(START_MAGIC);
    StreamHeader header;
    header.set_a4_version(2);
    header.set_description(description);
    header.set_metadata_refers_forward(_metadata_refers_forward);
    return write(_fixed_class_id<StreamHeader>(), header);
}

bool OutputStream::write_footer() {
    assert(!_compressed_out);
    StreamFooter footer;
    footer.set_size(get_bytes_written());
    for (uint32_t i = 0; i < metadata_positions.size(); i++)
        footer.add_metadata_offsets(metadata_positions[i]);
    for (uint32_t i = 0; i < protoclass_positions.size(); i++)
        footer.add_protoclass_offsets(protoclass_positions[i]);
    for (uint32_t i = 0; i < _class_id_counts.size(); i++) {
        if (_class_id_counts[i] > 0) {
            ClassCount* cc = footer.add_class_count();
            cc->set_class_id(i);
            cc->set_count(_class_id_counts[i]);
        }
    }
    write(_fixed_class_id<StreamFooter>(), footer);
    _coded_out->WriteLittleEndian32(footer.ByteSize());
    _coded_out->WriteString(END_MAGIC);
    return true;
}

/// Given a FileDescriptor, return all of the dependencies
/// in the file_descriptors set.
std::vector<const google::protobuf::FileDescriptor*> get_file_descriptors(
    std::set<const google::protobuf::FileDescriptor*>& seen_fds,
    const google::protobuf::FileDescriptor* file_descriptor)
{
    std::vector<const google::protobuf::FileDescriptor*> file_descriptors;
    if (!seen_fds.insert(file_descriptor).second)
        return file_descriptors;
        
    for (int i = 0; i < file_descriptor->dependency_count(); i++) {
        const auto tmp = get_file_descriptors(seen_fds, file_descriptor->dependency(i));
        file_descriptors.insert(file_descriptors.end(), tmp.begin(), tmp.end());
    }
    
    file_descriptors.push_back(file_descriptor);
    return file_descriptors;
}


/// Find all extension descriptors for `d`, including sub fields at any depth
/// which are message types.
/// Not handled: nested message types
std::vector<const google::protobuf::FieldDescriptor*> get_extension_descriptors(
    std::set<const google::protobuf::Descriptor*>& seen_descriptors,
    const google::protobuf::Descriptor* d)
{
    std::vector<const google::protobuf::FieldDescriptor*> extensions;
    if (!seen_descriptors.insert(d).second)
        return extensions;
    
    for (int i = 0; i < d->field_count(); i++) {
        const auto& field = *d->field(i);
        if (field.cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
            const auto tmp = get_extension_descriptors(seen_descriptors, field.message_type());
            extensions.insert(extensions.end(), tmp.begin(), tmp.end());
        }
    }
    
    d->file()->pool()->FindAllExtensions(d, &extensions);
    return extensions;
}

/// Write the protoclass definition to the output stream.
/// Strategy: Find all of the file descriptors needed to fully represent a class
/// (e.g. all of its field types). If any of the classes have been extended,
/// their extensions must be written out, too.
void OutputStream::write_protoclass(uint32_t class_id, const google::protobuf::Descriptor* d)
{
    ProtoClass a4proto;
    a4proto.set_class_id(class_id);
    a4proto.set_full_name(d->full_name());

    // Fetch file descriptors depended on by the file containing this class
    std::set<const google::protobuf::FileDescriptor*> seen_fds;
    auto file_descriptors = get_file_descriptors(seen_fds, d->file());
    
    // Fetch extensions on all subclasses of this descriptor
    std::set<const google::protobuf::Descriptor*> seen_descriptors;
    auto extensions = get_extension_descriptors(seen_descriptors, d);
    
    foreach (const google::protobuf::FieldDescriptor* ext, extensions) {
        const auto* fd = ext->file();
        if (!seen_fds.insert(fd).second)
            continue;
        file_descriptors.push_back(fd);
    }
    foreach (const google::protobuf::FileDescriptor* fd, file_descriptors) {
        if (!_written_file_descriptor_set.insert(fd->name()).second)
            continue; // Skip already written descriptors
        fd->CopyTo(a4proto.add_file_descriptor());
    }
    protoclass_positions.push_back(get_bytes_written());
    if (class_id >= _class_id_counts.size()) {
        _class_id_counts.resize(class_id+1);
    }
    write(_fixed_class_id<ProtoClass>(), a4proto);
}

bool OutputStream::write(uint32_t class_id, const google::protobuf::Message &msg)
{
    if (_coded_out->ByteCount() > 100000000) reset_coded_stream();
    _class_id_counts[class_id]++;

    string message;
    if (!msg.SerializeToString(&message))
        return false;

    uint32_t size = message.size();
    if (class_id == 0) {
        _coded_out->WriteLittleEndian32(size);
    } else {
        _coded_out->WriteLittleEndian32(size | HIGH_BIT );
        _coded_out->WriteLittleEndian32(class_id);
    }
    msg.SerializeWithCachedSizes(_coded_out.get());
    return true;
}

bool OutputStream::write(uint32_t class_id, shared<const A4Message> msg)
{
    if (_coded_out->ByteCount() > 100000000) reset_coded_stream();
    _class_id_counts[class_id]++;

    uint32_t size = msg->bytesize();
    if (class_id == 0) {
        _coded_out->WriteLittleEndian32(size);
    } else {
        _coded_out->WriteLittleEndian32(size | HIGH_BIT );
        _coded_out->WriteLittleEndian32(class_id);
    }
    /*
    if (not msg->_instream_read) {
        auto coded_in = msg->_coded_in.lock();
        int to_copy = msg->_size;
        const void* this_data = NULL;
        int this_step = 0;
        do {
            if (not coded_in->GetDirectBufferPointer(&this_data, &this_step)) {
                return false;
            }
            assert(this_step > 0);
            this_step = this_step > to_copy ? to_copy : this_step;
            _coded_out->WriteRaw(this_data, this_step);
            to_copy -= this_step;
        } while (to_copy > 0);
        msg->_coded_in.reset(); // invalidate message
        msg->_instream_read = true;
    } else {
        _coded_out->WriteString(msg->bytes());
    }*/
    _coded_out->WriteString(msg->bytes());
    return true;
}


