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
        _descriptor_pool.reset(new DescriptorPool(DescriptorPool::generated_pool()));
        _message_factory.reset(new DynamicMessageFactory(_descriptor_pool.get()));
    }

    ProtoClassPool::~ProtoClassPool() {
        _message_factory.reset();
        _descriptor_pool.reset();
    }

    A4Message ProtoClassPool::read(uint32_t class_id, google::protobuf::io::CodedInputStream* instream) {
        if (class_id < _class_id_reader.size()) {
            internal::from_stream_func factory = _class_id_reader[class_id];
            if (factory) {
                return A4Message(class_id,
                                 _class_id_reader[class_id](instream),
                                 _class_id_descriptor[class_id],
                                 _dynamic_descriptor[class_id],
                                 _descriptor_pool,
                                 _message_factory
                                 );
            }
        }
        // Look for fixed class_id message
        internal::classreg reg = internal::map_class("", class_id);
        if (!reg.descriptor) throw a4::Fatal("Unregistered class_id: ", class_id);
        if (class_id >= _class_id_reader.size()) {
            _class_id_reader.resize(class_id+1);
            _class_id_descriptor.resize(class_id+1);
            _dynamic_descriptor.resize(class_id+1);
            _class_id_reader[class_id] = reg.from_stream;
            _class_id_descriptor[class_id] = reg.descriptor;
            _dynamic_descriptor[class_id] = NULL;
        }
        return A4Message(class_id, reg.from_stream(instream), reg.descriptor);
    };

    void ProtoClassPool::add_protoclass(const ProtoClass & protoclass) {
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
            std::string fqn = fdp.package() + "." + fdp.name();
            //std::cout << "Reading in file: " << fqn << std::endl;
            assert(_encountered_file_descriptors.count(fqn) == 0);
            _encountered_file_descriptors.insert(fqn);
            _descriptor_pool->BuildFile(fdp);
        }

        // First, find the generated descriptor
        //std::cout << "Looking for " << protoclass.full_name() << std::endl;
        const Descriptor * gd = _descriptor_pool->FindMessageTypeByName(protoclass.full_name());
        assert(gd);

        // Now try to find an compiled-in function for the class
        internal::classreg reg = internal::map_class(protoclass.full_name());
        if (reg.descriptor) {
            std::string cmajor = reg.descriptor->options().GetExtension(major_version);
            std::string cminor = reg.descriptor->options().GetExtension(minor_version);
            std::string dmajor = gd->options().GetExtension(major_version);
            std::string dminor = gd->options().GetExtension(minor_version);
            if (cmajor != dmajor) {
                throw a4::Fatal("Major versions of compiled-in and read messages do not agree:",
                                "Compiled in: '", cmajor, "', Read: '", dmajor, "'");
            } else if (cminor != dminor) {
                std::cerr << "Warning: Minor versions of compiled-in and read messages do not agree:" <<
                             "Compiled in: '" << cminor << "' << Read: '" << dminor << "'" << std::endl;
            }
            _class_id_reader[class_id] = reg.from_stream;
            _class_id_descriptor[class_id] = reg.descriptor;
            _dynamic_descriptor[class_id] = gd;
        } else { // Use dynamic reading
            //std::cerr << "Warning: No compiled version of " << protoclass.full_name() << " found!" << std::endl;
            const Message* prototype = _message_factory->GetPrototype(gd);
            _class_id_reader[class_id] = bind(&ProtoClassPool::message_factory, this, prototype, _1);
            _class_id_descriptor[class_id] = gd;
            _dynamic_descriptor[class_id] = gd;
        }
    }

    shared<Message> ProtoClassPool::message_factory(
        const google::protobuf::Message* prototype,
        google::protobuf::io::CodedInputStream* instr)
    {
        shared<Message> msg(prototype->New());
        bool success = msg->ParseFromCodedStream(instr);
        assert(success);
        return msg;
    }


};};

