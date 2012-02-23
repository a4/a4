#include <boost/bind.hpp>
using boost::bind;

#include "proto_class_pool.h"

using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::FileDescriptorProto;
using google::protobuf::FileDescriptor;

namespace a4{ namespace io{

    ProtoClassPool::ProtoClassPool() {
        // This descriptor pool must be kept alive as long as messages
        // generated in this stream block are around (one might need to
        // dynamically merge metadata, for example)
        _descriptor_pool.reset(new DescriptorPool());
        _message_factory.reset(new DynamicMessageFactory(_descriptor_pool.get()));
    }

    ProtoClassPool::~ProtoClassPool() {
        _message_factory.reset();
        _descriptor_pool.reset();
    }
    
    void ProtoClassPool::verify_class_id(uint32_t class_id) {
        if (class_id < _class_id_reader.size()) {
            if (_class_id_reader[class_id]) return;
        }
        // Look for fixed class_id message
        internal::classreg reg = internal::map_class("", class_id);
        if (!reg.descriptor)
            FATAL("Unregistered class_id: ", class_id);
        
        if (class_id >= _class_id_reader.size()) {
            _class_id_reader.resize(class_id+1);
            _class_id_descriptor.resize(class_id+1);
            _dynamic_descriptor.resize(class_id+1);
        }
        _class_id_reader[class_id] = reg.new_protoclass;
        _class_id_descriptor[class_id] = reg.descriptor;
        _dynamic_descriptor[class_id] = NULL;
        assert(reg.new_protoclass);
    }

    shared<google::protobuf::Message> ProtoClassPool::get_new_message(uint32_t class_id) {
        verify_class_id(class_id);
        return  _class_id_reader[class_id]();
    }
    
    shared<google::protobuf::Message> ProtoClassPool::get_new_message(const Descriptor* d) const {
        return shared<google::protobuf::Message>(_message_factory->GetPrototype(d)->New());
    }

    shared<google::protobuf::Message> ProtoClassPool::parse_message(uint32_t class_id, 
                                   shared<google::protobuf::io::CodedInputStream> coded_in,
                                   size_t size)
    {
        assert(coded_in);
        //std::cerr << "Parse from stream " << coded_in.get() << "\tSZ " << size << "\tID " << class_id << std::endl;
        auto msg = get_new_message(class_id);
        auto lim = coded_in->PushLimit(size);
        if (not msg->ParseFromCodedStream(coded_in.get())) {
            FATAL("Could not parse from stream!");
            coded_in->PushLimit(0); // Prevent the stream from reading further
            return shared<google::protobuf::Message>();
        }
        coded_in->PopLimit(lim);
        return msg;
    }

    shared<google::protobuf::Message> ProtoClassPool::parse_message(uint32_t class_id, 
            const std::string& bytes)
    {
        //std::cerr << "Parse from string\tSZ" << bytes.size() << "\tID " << class_id << std::endl;
        auto msg = get_new_message(class_id);
        if (!msg->ParseFromString(bytes)) {
            return shared<google::protobuf::Message>();
        }
        return msg;
    }

    const Descriptor* ProtoClassPool::descriptor(uint32_t class_id) {
        verify_class_id(class_id);
        auto d = _class_id_descriptor[class_id];
        assert(d);
        return d;
    }

    const Descriptor* ProtoClassPool::dynamic_descriptor(uint32_t class_id) {
        verify_class_id(class_id);
        return _dynamic_descriptor[class_id];
    }

#if 0
    shared<A4Message> ProtoClassPool::read(uint32_t class_id,
                                   google::protobuf::io::CodedInputStream* instream,
                                   size_t size)
    {


        if (class_id < _class_id_reader.size()) {
            internal::new_protoclass_func factory = _class_id_reader[class_id];
            if (factory) {
                return A4Message(class_id,
                                 _class_id_reader[class_id],
                                 instream, size,
                                 _class_id_descriptor[class_id],
                                 _dynamic_descriptor[class_id],
                                 _descriptor_pool
                                 );
            }
        }
        // Look for fixed class_id message
        internal::classreg reg = internal::map_class("", class_id);
        if (!reg.descriptor) 
            FATAL("Unregistered class_id: ", class_id);
        
        if (class_id >= _class_id_reader.size()) {
            _class_id_reader.resize(class_id+1);
            _class_id_descriptor.resize(class_id+1);
            _dynamic_descriptor.resize(class_id+1);
            _class_id_reader[class_id] = reg.new_protoclass;
            _class_id_descriptor[class_id] = reg.descriptor;
            _dynamic_descriptor[class_id] = NULL;
        }
        assert(reg.new_protoclass);
        return A4Message(class_id, reg.new_protoclass, instream, size, 
                          reg.descriptor, NULL, _descriptor_pool);
    }
#endif

    void ProtoClassPool::add_protoclass(const ProtoClass& protoclass) {
        uint32_t class_id = protoclass.class_id();
        if (class_id >= _class_id_reader.size()) {
            _class_id_reader.resize(class_id+1);
            _class_id_descriptor.resize(class_id+1);
            _dynamic_descriptor.resize(class_id+1);
        }

        // If we've already seen this protoclass, ignore it.
        if(_class_id_reader[class_id] != NULL) return;

        // First, add all file descriptors to our pool
        foreach(const FileDescriptorProto& fdp, protoclass.file_descriptor()) {
            // Check if we already have this FD (unneccessary, should never be the case)
            assert(_encountered_file_descriptors.count(fdp.name()) == 0);
            _encountered_file_descriptors.insert(fdp.name());
            _descriptor_pool->BuildFile(fdp);
        }

        // First, find the generated descriptor
        //std::cout << "Looking for " << protoclass.full_name() << std::endl;
        const Descriptor* gd = _descriptor_pool->FindMessageTypeByName(protoclass.full_name());
        assert(gd);

        // Now try to find an compiled-in function for the class
        internal::classreg reg = internal::map_class(protoclass.full_name());
        if (reg.descriptor) {
            std::string cmajor = reg.descriptor->options().GetExtension(major_version);
            std::string cminor = reg.descriptor->options().GetExtension(minor_version);
            std::string dmajor = gd->options().GetExtension(major_version);
            std::string dminor = gd->options().GetExtension(minor_version);
            if (cmajor != dmajor) {
                FATAL("Major versions of compiled-in and read messages do not agree:",
                      "Compiled in: '", cmajor, "', Read: '", dmajor, "'");
            } else if (cminor != dminor) {
                WARNING("Minor versions of compiled-in and read messages do not agree: "
                        "Compiled in: '", cminor, "' << Read: '", dminor, "'");
            }
            _class_id_reader[class_id] = reg.new_protoclass;
            _class_id_descriptor[class_id] = reg.descriptor;
            _dynamic_descriptor[class_id] = gd;
        } else { // Use dynamic reading
            // WARNING("No compiled version of ", protoclass.full_name(), " found!");
            const Message* prototype = _message_factory->GetPrototype(gd);
            _class_id_reader[class_id] = bind(&ProtoClassPool::new_protoclass, this, prototype);
            _class_id_descriptor[class_id] = gd;
            _dynamic_descriptor[class_id] = gd;
        }
    }

    shared<Message> ProtoClassPool::new_protoclass(
        const google::protobuf::Message* prototype)
    {
        return shared<Message>(prototype->New());
    }

};};

