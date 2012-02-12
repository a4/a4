#ifndef _A4_PROTO_CLASS_POOL_H_
#define _A4_PROTO_CLASS_POOL_H_

#include <string>
#include <vector>
#include <set>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/types.h>
#include <a4/register.h>
#include <a4/message.h>

#include <A4Stream.pb.h>

namespace a4{ namespace io{

    /// Keeps track of ProtoClass classes and Metadata classes and offsets in a single
    /// block between Header and Footer.
    class ProtoClassPool {
        public:
            ProtoClassPool();
            ~ProtoClassPool();
            A4Message read(uint32_t class_id, google::protobuf::io::CodedInputStream* instream);
            void add_protoclass(const ProtoClass& protoclass);
            shared<Message> message_factory(const google::protobuf::Message* prototype, google::protobuf::io::CodedInputStream* instr);

        private:
            std::set<std::string> _encountered_file_descriptors;
            std::vector<internal::from_stream_func> _class_id_reader;
            std::vector<const google::protobuf::Descriptor*> _class_id_descriptor;
            std::vector<const google::protobuf::Descriptor*> _dynamic_descriptor;
            shared<google::protobuf::DescriptorPool> _descriptor_pool;
            shared<google::protobuf::DynamicMessageFactory> _message_factory;
    };

};};

#endif
