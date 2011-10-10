#define _FILE_OFFSET_BITS 64

#include <config.h>

#include <iostream>

#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>
using boost::bind;
using boost::function;

#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "gzip_stream.h"
#ifdef HAVE_SNAPPY
#include "snappy_stream.h"
#endif
#include "a4/proto/io/A4Stream.pb.h"
#include "a4/input_stream.h"

using std::string;
using std::cerr;
using std::endl;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::FileInputStream;
using google::protobuf::io::CodedInputStream;

using google::protobuf::DynamicMessageFactory;
using google::protobuf::SimpleDescriptorDatabase;
using google::protobuf::DescriptorPool;

using google::protobuf::FileDescriptorProto;
using google::protobuf::FileDescriptor;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Reflection;

using namespace a4::io;

const string START_MAGIC = "A4STREAM";
const string END_MAGIC = "KTHXBYE4";
const int START_MAGIC_len = 8;
const int END_MAGIC_len = 8;
const uint32_t HIGH_BIT = 1<<31;

A4InputStream::A4InputStream(const string &input_file) {
    _started = false;
    _fileno = -1;
    _inputname = input_file;
    init();
}

A4InputStream::A4InputStream(shared<ZeroCopyInputStream> in, std::string name) {
    _started = false;
    _fileno = 0;
    _raw_in = in;
    _inputname = name;
    init();
}

A4Message A4InputStream::set_error() {
    _error = true;
    _good = false;
    return A4Message(true);
}

A4Message A4InputStream::set_end() {
    _good = false;
    return A4Message();
}

void A4InputStream::init() {
    _started = false;
    _compressed_in.reset();
    _coded_in.reset();
    _good = true;
    _error = false;
    _new_metadata = false;
    _discovery_complete = false;
    _items_read = 0;
    _content_class_id = 0;
    _metadata_class_id = 0;
    _content_func = &a4::io::internal::bad_from_stream_func;
    _current_metadata_refers_forward = false;
    _current_header_index = 0;
    _current_metadata_index = 0;
    _encountered_file_descriptors.reset(new SimpleDescriptorDatabase());
    //_encountered_file_descriptors.get()
    _descriptor_pool.reset(new DescriptorPool(DescriptorPool::generated_pool()));
    _message_factory.reset(new DynamicMessageFactory(_descriptor_pool.get()));
    // If the type is compiled in then use that! (urk, no worky? deprecated?)
    _message_factory->SetDelegateToGeneratedFactory(true);
}

void A4InputStream::startup() {
    // Initialize to defined state
    _started = true;

    if (_fileno == -1) {
        _fileno = open(_inputname.c_str(), O_RDONLY);
        _file_in.reset(new FileInputStream(_fileno));
        _raw_in = _file_in;
    } else {
        _file_in.reset();
    }
    _coded_in.reset(new CodedInputStream(_raw_in.get()));

    // Push limit of read bytes
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));

    if (!read_header()) {
        if(_error) std::cerr << "ERROR - a4::io:A4InputStream - Header corrupted!" << std::endl;
        else std::cerr << "ERROR - a4::io:A4InputStream - File empty!" << std::endl;
        set_error();
        return;
    }
    if (_file_in && _file_in->GetErrno()) {
        std::cerr << "ERROR - a4::io:A4InputStream - Could not open '"<< _inputname \
                  << "' - error " << _file_in->GetErrno() << std::endl;
        set_error();
        return;
    }
    _current_header_index = 0;
}

A4InputStream::~A4InputStream() {
    _coded_in.reset();
    _compressed_in.reset();
    _raw_in.reset();
    _file_in.reset();
}

bool A4InputStream::read_header()
{
    // Note: in the following i use that bool(set_end()) == false 
    // && bool(set_error()) == false
    string magic;
    if (!_coded_in->ReadString(&magic, 8))
        return set_end();

    if (0 != magic.compare(START_MAGIC))
        return set_error();

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size))
        return set_error();

    uint32_t message_type = 0;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type))
            return set_error();
    }
    if (!message_type == A4StreamHeader::kCLASSIDFieldNumber)
        return set_error();

    A4StreamHeader h;
    CodedInputStream::Limit lim = _coded_in->PushLimit(size);
    if (!h.ParseFromCodedStream(_coded_in.get()))
        return set_error();
    _coded_in->PopLimit(lim);
    
    if (h.a4_version() != 1) {
        std::cerr << "ERROR - a4::io:A4InputStream - Unknown A4 stream version (";
        std::cerr << h.a4_version() << ")" << std::endl;
        return set_error();
    }

    // check the main content type from the header
    if (h.has_content_class_id()) {
        _content_class_id = h.content_class_id();
        _content_func = internal::all_class_ids(_content_class_id, NULL, false);
        if (!_content_func) {
            std::cerr << "WARNING - a4::io:A4InputStream - Content Class " << _content_class_id << " unknown!" << std::endl;
            //return set_error();
        }
    } else {
        _content_class_id = 0;
    }
    
    if (h.has_metadata_class_id()) {
        _metadata_class_id = h.metadata_class_id();
        if (internal::all_class_ids(_metadata_class_id) == &a4::io::internal::bad_from_stream_func) {
            std::cerr << "WARNING - a4::io:A4InputStream - metadata Class " << _metadata_class_id << " unknown!" << std::endl;
            //return set_error();
        }
    } else {
        _metadata_class_id = 0;
    }

    _current_metadata_refers_forward = h.metadata_refers_forward();
    if (!_current_metadata_refers_forward && !_discovery_complete) {
        if (!_file_in) {
            std::cerr << "ERROR - a4::io:A4InputStream - Cannot read reverse metadata from non-seekable stream!" << std::endl;
            return set_error();
        } else if (!discover_all_metadata()) {
            std::cerr << "ERROR - a4::io:A4InputStream - Failed to discover metadata - file corrupted?" << std::endl;
            return set_error();
        }
        _current_metadata_index = 0;
        if (_metadata_per_header[_current_header_index].size() > 0) {
            _current_metadata = _metadata_per_header[_current_header_index][0];
            _new_metadata = true;
        }
    }
    return true;
}

bool A4InputStream::discover_all_metadata() {
    assert(_metadata_per_header.size() == 0);

    int64_t size = 0;
    std::deque<uint64_t> headers;

    while (true) {
        if (seek(-size - END_MAGIC_len, SEEK_END) == -1) return false;
        string magic;
        if (!_coded_in->ReadString(&magic, 8)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file during scan!" << std::endl; 
            return false;
        }
        if (seek(-size - END_MAGIC_len - 4, SEEK_END) == -1) return false;

        uint32_t footer_size = 0;
        if (!_coded_in->ReadLittleEndian32(&footer_size)) return false;
        uint32_t footer_msgsize  = END_MAGIC_len + 4 + footer_size + 8;
        uint64_t footer_start = - size - footer_msgsize;
        int64_t footer_abs_start = seek(footer_start, SEEK_END);
        if (footer_abs_start == -1) return false;
        A4Message msg = next(true);
        if (!msg.is<A4StreamFooter>()) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unknown footer class_id " << msg.class_id << std::endl;
            return false;
        }
        shared<A4StreamFooter> footer = msg.as<A4StreamFooter>();
        size += footer->size() + footer_msgsize;

        std::vector<A4Message> _this_headers_metadata;
        foreach(uint64_t offset, footer->metadata_offsets()) {
            uint64_t metadata_start = footer_abs_start - footer->size() + offset;
            if (seek(metadata_start, SEEK_SET) == -1) return false;
            A4Message msg = next(true);
            
            while (msg.class_id == uint32_t(A4Proto::kCLASSIDFieldNumber)) {
                // Instead of getting metadata we might get class descriptions here
                shared<A4Proto> a4proto = msg.as<A4Proto>();
                generate_dynamic_classes(a4proto.get());
                msg = next(true);
            }
            
            if (msg.class_id != _metadata_class_id) {
                std::cerr << "ERROR - a4::io:A4InputStream - class_id is not metadata class_id: "
                          << msg.class_id << " != " << _metadata_class_id << std::endl;
                return false;
            }
            _this_headers_metadata.push_back(msg);
        }
        _metadata_per_header.push_front(_this_headers_metadata);

        int64_t tell = seek(-size, SEEK_END);
        headers.push_front(tell);
        if (tell == -1) return false;
        if (tell == 0) break;
    }
    seek(headers[_current_header_index] + START_MAGIC_len, SEEK_SET);
    next(true);
    _discovery_complete = true;
    return true;
}

int64_t A4InputStream::seek(int64_t position, int whence) {
    assert(_fileno != 0);
    assert(!_compressed_in);
    _coded_in.reset();
    _file_in.reset();
    _raw_in.reset();
    int64_t pos = lseek(_fileno, position, whence);
    _file_in.reset(new FileInputStream(_fileno));
    _raw_in = _file_in;
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return pos;
};

bool A4InputStream::start_compression(const A4StartCompressedSection& cs) {
    assert(!_compressed_in);
    _coded_in.reset();

    if (cs.compression() == A4StartCompressedSection_Compression_ZLIB) {
        _compressed_in.reset(new GzipInputStream(_raw_in.get(), GzipInputStream::ZLIB));
    } else if (cs.compression() == A4StartCompressedSection_Compression_GZIP) {
        _compressed_in.reset(new GzipInputStream(_raw_in.get(), GzipInputStream::GZIP));
    } else if (cs.compression() == A4StartCompressedSection_Compression_SNAPPY) {
#ifdef HAVE_SNAPPY
        _compressed_in.reset(new SnappyInputStream(_raw_in.get()));
#else
        throw a4::Fatal("This file uses compression by the 'Snappy' library, which was not compiled in!");
#endif
    } else {
        std::cerr << "ERROR - a4::io:A4InputStream - Unknown compression type " << cs.compression() << std::endl;
        return false;
    }

    _coded_in.reset(new CodedInputStream(_compressed_in.get()));
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
    return true;
}

bool A4InputStream::stop_compression(const A4EndCompressedSection& cs) {
    assert(_compressed_in);
    _coded_in.reset();
    if (!_compressed_in->ExpectAtEnd()) {
        std::cerr << "ERROR - a4::io:A4InputStream - Compressed section did not end where it should!" << std::endl;
        return false;
    }
    _compressed_in.reset();
    _coded_in.reset(new CodedInputStream(_raw_in.get()));
    return true;
}

void A4InputStream::reset_coded_stream() {
    _coded_in.reset();
    if (_compressed_in) {
        _coded_in.reset(new CodedInputStream(_compressed_in.get()));
    } else {
        _coded_in.reset(new CodedInputStream(_raw_in.get()));
    }
    _coded_in->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
}

shared<Message> A4InputStream::message_factory(
    const google::protobuf::Message* prototype,
    google::protobuf::io::CodedInputStream* instr)
{    
    shared<Message> msg(prototype->New());
    bool success = msg->ParseFromCodedStream(instr);
    assert(success);
    return msg;
}

void A4InputStream::generate_dynamic_classes(const A4Proto* a4proto)
{
    const FileDescriptorProto& fdp = a4proto->file_descriptor();
    //std::cout << "Encountered a dynamic class: " << fdp.name() << std::endl;
    
    //std::cout << "Encountered an A4Proto on reading: " << fdp.name() << std::endl;
    //std::cout << "Number of messages: " << fdp.message_type_size() << std::endl;
    
    {
        google::protobuf::LogSilencer silencer;
        if (not _encountered_file_descriptors->Add(fdp))
            return;
    }
    
    const FileDescriptor* file = _descriptor_pool->BuildFile(fdp);
    
    for (int i = 0; i < file->message_type_count(); i++) {
        const Descriptor* d = file->message_type(i);
        const FieldDescriptor* fd = d->FindFieldByName("CLASS_ID");
        
        if (!fd)
            continue; // No CLASS_ID field, ignore this type

        // Bingo, we found a A4 message type
        const uint32_t class_id = fd->number();

        _descriptor_map[class_id] = d;

        if (a4::io::internal::all_class_ids(class_id, NULL, false) == &a4::io::internal::bad_from_stream_func) {
            // No handler available, make a factory for it
            
            const Message* prototype = _message_factory->GetPrototype(d);
            boost::function<shared<Message> (google::protobuf::io::CodedInputStream*)> factory = 
                bind(&A4InputStream::message_factory, this, prototype, _1);
                
            a4::io::internal::all_class_ids(class_id, factory);
            
            if (class_id == _content_class_id) _content_func = factory;
        }
    }
    
    // This dumps the protobuf
    // if (file) std::cout << file->DebugString() << std::endl;
}

const Descriptor* A4InputStream::dynamic_descriptor(uint32_t class_id) {
    std::map<uint32_t, const Descriptor*>::const_iterator d = _descriptor_map.find(class_id);
    if (d == _descriptor_map.end()) return NULL;
    return d->second;
}

typedef boost::variant<int32_t, int64_t, uint32_t, uint64_t, double, float, bool, std::string> FieldContent;

FieldContent get_repeated_field(const Message & m, int field_number, int i) {
    const Reflection * r = m.GetReflection();
    const FieldDescriptor * f = m.GetDescriptor()->FindFieldByNumber(field_number);
    switch (f->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32: return r->GetRepeatedInt32(m, f, i);
        case FieldDescriptor::CPPTYPE_INT64: return r->GetRepeatedInt64(m, f, i);
        case FieldDescriptor::CPPTYPE_UINT32: return r->GetRepeatedUInt32(m, f, i);
        case FieldDescriptor::CPPTYPE_UINT64: return r->GetRepeatedUInt64(m, f, i);
        case FieldDescriptor::CPPTYPE_DOUBLE: return r->GetRepeatedDouble(m, f, i);
        case FieldDescriptor::CPPTYPE_FLOAT: return r->GetRepeatedFloat(m, f, i);
        case FieldDescriptor::CPPTYPE_BOOL: return (uint32_t)r->GetRepeatedBool(m, f, i);
        case FieldDescriptor::CPPTYPE_ENUM: return (uint32_t)r->GetRepeatedEnum(m, f, i)->number();
        case FieldDescriptor::CPPTYPE_STRING: return r->GetRepeatedString(m, f, i);
        case FieldDescriptor::CPPTYPE_MESSAGE: return r->GetRepeatedMessage(m, f, i).DebugString();
    };
    throw a4::Fatal("Unknown type ", f->cpp_type());
}

FieldContent get_field(const Message & m, int field_number) {
    const Reflection * r = m.GetReflection();
    const FieldDescriptor * f = m.GetDescriptor()->FindFieldByNumber(field_number);
    switch (f->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32: return r->GetInt32(m, f);
        case FieldDescriptor::CPPTYPE_INT64: return r->GetInt64(m, f);
        case FieldDescriptor::CPPTYPE_UINT32: return r->GetUInt32(m, f);
        case FieldDescriptor::CPPTYPE_UINT64: return r->GetUInt64(m, f);
        case FieldDescriptor::CPPTYPE_DOUBLE: return r->GetDouble(m, f);
        case FieldDescriptor::CPPTYPE_FLOAT: return r->GetFloat(m, f);
        case FieldDescriptor::CPPTYPE_BOOL: return (uint32_t)r->GetBool(m, f);
        case FieldDescriptor::CPPTYPE_ENUM: return (uint32_t)r->GetEnum(m, f)->number();
        case FieldDescriptor::CPPTYPE_STRING: return r->GetString(m, f);
        case FieldDescriptor::CPPTYPE_MESSAGE: return r->GetMessage(m, f).DebugString();
    };
    throw a4::Fatal("Unknown type ", f->cpp_type());
}

bool messages_field_equal(const Message & m1, const Message & m2, int field_number) {
    const FieldDescriptor * fd1 = m1.GetDescriptor()->FindFieldByNumber(field_number);
    const FieldDescriptor * fd2 = m1.GetDescriptor()->FindFieldByNumber(field_number);
    assert(fd1 && fd2);
    if (fd1->is_repeated()) {
        int size1 = m1.GetReflection()->FieldSize(m1, fd1);
        int size2 = m2.GetReflection()->FieldSize(m2, fd2);
        assert(size1 == size2);
        for (int i = 0; i < size1; i++) {
            if (!(get_repeated_field(m1, field_number, i) == 
                  get_repeated_field(m2, field_number, i))) return false;
        }
        return true;
    }
    return get_field(m1, field_number) == get_field(m2, field_number);
}

unique<Message> A4InputStream::merge_messages(A4Message m1, A4Message m2) {
    assert(m1.class_id == m2.class_id);
    const Descriptor * d = dynamic_descriptor(m1.class_id);
    assert(d != NULL);
    unique<google::protobuf::Message> merged(_message_factory->GetPrototype(d)->New());
    for (int i = 0; i < d->field_count(); i++) {
        const google::protobuf::FieldOptions & opt = d->field(i)->options();
        const Reflection * ref = opt.GetReflection();
        assert(ref);
        const FieldDescriptor * fdm = ref->FindKnownExtensionByNumber(kMergeFieldNumber);
        const google::protobuf::EnumValueDescriptor * evd = ref->GetEnum(opt, fdm);

        Message & msg1 = *m1.message;
        Message & msg2 = *m2.message;
        switch(evd->number()) {
            case MERGE_BLOCK_IF_DIFFERENT:
                if (!messages_field_equal(msg1, msg2, d->field(i)->number())) {
                    throw a4::Fatal("Trying to merge two metadata objects with different ", d->field(i)->full_name());
                }
                break;
            case MERGE_ADD:
                std::cerr << " A " << d->field(i)->full_name() << std::endl;
                break;
            case MERGE_UNION:
                std::cerr << " U " << d->field(i)->full_name() << std::endl;
                break;
            default:
                throw a4::Fatal("Unknown merge strategy: ", evd->name(), ". Recompilation should fix it.");
        }
    }
    return merged;
}


/// \internal
/// Do not do any processing on the message if internal=true
/// \endinternal
A4Message A4InputStream::next(bool internal) {
    
    if (!_started) startup();

    if (!_good) return A4Message(_error);

    static int i = 0;
    if (i++ % 100 == 0) reset_coded_stream();

    uint32_t size = 0;
    if (!_coded_in->ReadLittleEndian32(&size)) {
        if (_compressed_in && _compressed_in->ByteCount() == 0) {
            std::cerr << "ERROR - a4::io:A4InputStream - Reading from compressed section failed!" << std::endl; 
        } else {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file or corruption [0]!" << std::endl; 
        }
        return set_error();
    }

    uint32_t message_type = _content_class_id;
    if (size & HIGH_BIT) {
        size = size & (HIGH_BIT - 1);
        if (!_coded_in->ReadLittleEndian32(&message_type)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file [1]!" << std::endl; 
            return set_error();
        }
    } else if (message_type == 0) {
        std::cerr << "ERROR - a4::io:A4InputStream - Default Object found while no default type is set!" << std::endl; 
        return set_error();
    }

    string magic;

    if (message_type == _content_class_id) {
        CodedInputStream::Limit lim = _coded_in->PushLimit(size);
        shared<Message> item = _content_func(_coded_in.get());
        _coded_in->PopLimit(lim);
        if (!item) {
            std::cerr << "ERROR - a4::io:A4InputStream - Failure to parse Item!" << std::endl;
            return set_error();
        }
        ++_items_read;
        return A4Message(message_type, item);
    }

    internal::from_stream_func from_stream = internal::all_class_ids(message_type);
    if (from_stream == &a4::io::internal::bad_from_stream_func) {
        std::cerr << "WARNING - a4::io:A4InputStream - Unknown message type: " << message_type << std::endl;
        if (!_coded_in->Skip(size)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Read error while skipping unknown message!"  << std::endl;
            return set_error();
        }
        return next();
    }
    CodedInputStream::Limit lim = _coded_in->PushLimit(size);
    shared<Message> item = from_stream(_coded_in.get());
    _coded_in->PopLimit(lim);
    if (!item) {
        std::cerr << "ERROR - a4::io:A4InputStream - Failure to parse object!" << std::endl;
        return set_error();
    }
    
    if (internal) return A4Message(message_type, item);
        
    if (message_type == uint32_t(A4StreamFooter::kCLASSIDFieldNumber)) {
        // TODO: Process footer
        shared<A4StreamFooter> foot = static_pointer_cast<A4StreamFooter>(item);

        if (!_coded_in->ReadLittleEndian32(&size)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file [3]!" << std::endl; 
            return set_error();
        }
        if (!_coded_in->ReadString(&magic, 8)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Unexpected end of file [4]!" << std::endl; 
            return set_error();
        }
        if (0 != magic.compare(END_MAGIC)) {
            std::cerr << "ERROR - a4::io:A4InputStream - Corrupt footer! Read: "<< magic << std::endl; 
            return set_error();
        }
        if (_coded_in->ExpectAtEnd()) {
            return set_end();
        }
        _current_header_index++;
        if (!read_header()) {
            if (_error) {
                std::cerr << "ERROR - a4::io:A4InputStream - Corrupt header!" << std::endl;
            }
            return A4Message(_error);
        }
        _current_metadata = A4Message();
        if (!_current_metadata_refers_forward) {
            _current_metadata_index = 0;
            if (_metadata_per_header[_current_header_index].size() > 0)
                _current_metadata = _metadata_per_header[_current_header_index][0];
        }
        return next();
    } else if (message_type == uint32_t(A4StreamHeader::kCLASSIDFieldNumber)) {
        // should not happen???
        std::cerr << "ERROR - a4::io:A4InputStream - Unexpected header!" << std::endl; 
        return set_error();
    } else if (message_type == uint32_t(A4StartCompressedSection::kCLASSIDFieldNumber)) {
        if (!start_compression(*static_cast<A4StartCompressedSection*>(item.get()))) {
            return set_error();
        }
        return next();
    } else if (message_type == uint32_t(A4EndCompressedSection::kCLASSIDFieldNumber)) {
        if(!stop_compression(*static_cast<A4EndCompressedSection*>(item.get()))) {
            return set_error(); // read error;
        }
        return next();
    } else if (message_type == uint32_t(A4Proto::kCLASSIDFieldNumber)) {
        shared<A4Proto> a4proto = static_pointer_cast<A4Proto>(item);
        generate_dynamic_classes(a4proto.get());
        return next();
    } else if (message_type == _metadata_class_id) {
        if (_current_metadata_refers_forward) {
            _current_metadata = A4Message(_metadata_class_id, item);
        } else {
            _current_metadata_index++;
            _current_metadata = A4Message();
            if (_metadata_per_header[_current_header_index].size() > _current_metadata_index)
                _current_metadata = _metadata_per_header[_current_header_index][_current_metadata_index];
        }
        _new_metadata = true;
        return next();
    }
    return A4Message(message_type, item);
}

