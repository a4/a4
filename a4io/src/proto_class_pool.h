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

#include <a4/io/A4Stream.pb.h>

namespace a4{ namespace io{

    /// Keeps track of ProtoClass classes and Metadata classes and offsets in a single
    /// block between Header and Footer.
    class ProtoClassPool {
        public:
            ProtoClassPool();
            ~ProtoClassPool();
            //shared<A4Message> read(uint32_t class_id, google::protobuf::io::CodedInputStream* instream, size_t size);
            void add_protoclass(const ProtoClass& protoclass);
            void verify_class_id(uint32_t class_id);
            shared<google::protobuf::Message> get_new_message(uint32_t class_id);
            shared<google::protobuf::Message> get_new_message(const google::protobuf::Descriptor* d) const;
            shared<google::protobuf::Message> parse_message(uint32_t class_id, 
                                   shared<google::protobuf::io::CodedInputStream> coded_in,
                                           size_t size);
            shared<google::protobuf::Message> parse_message(uint32_t class_id, 
                                            const std::string& bytes);
            const google::protobuf::Descriptor* descriptor(uint32_t class_id);
            const google::protobuf::Descriptor* dynamic_descriptor(uint32_t class_id);
            
            std::vector<const google::protobuf::FileDescriptor*> get_filedescriptors() {
                std::vector<const google::protobuf::FileDescriptor*> result;
                foreach (auto& fd_name, _encountered_file_descriptors)
                    result.push_back(_descriptor_pool->FindFileByName(fd_name));
                return result;
            }
        
        private:
            shared<Message> new_protoclass(const google::protobuf::Message* prototype);
            std::set<std::string> _encountered_file_descriptors;
            std::vector<internal::new_protoclass_func> _class_id_reader;
            std::vector<const google::protobuf::Message*> _class_id_prototype;
            std::vector<const google::protobuf::Descriptor*> _class_id_descriptor;
            std::vector<const google::protobuf::Descriptor*> _dynamic_descriptor;
            shared<google::protobuf::DescriptorPool> _descriptor_pool;
            shared<google::protobuf::DynamicMessageFactory> _message_factory;
    };

};};

#endif
