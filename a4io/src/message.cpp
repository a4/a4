#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/message.h>

#include <A4.pb.h>

#include "dynamic_message.h"

namespace a4{ namespace io{

    shared<Message> A4Message::as_dynamic_message(const google::protobuf::Descriptor* d, const google::protobuf::DescriptorPool* p) const {

        using google::protobuf::DynamicMessageFactory;

        if (d == _descriptor) return message;
        unique<DynamicMessageFactory> _message_factory(new DynamicMessageFactory(p));
        shared<Message> m(_message_factory->GetPrototype(d)->New());
        // Do version checking if the dynamic descriptors are different
        if (d != _dynamic_descriptor) {
            const google::protobuf::Descriptor* my_d = _dynamic_descriptor ? _dynamic_descriptor : _descriptor;

            std::string mymajor = my_d->options().GetExtension(major_version);
            std::string myminor = my_d->options().GetExtension(minor_version);
            std::string dmajor = d->options().GetExtension(major_version);
            std::string dminor = d->options().GetExtension(minor_version);

            if (mymajor != dmajor) {
                throw a4::Fatal("Major versions of objects to merge do not agree:", mymajor, " != ", dmajor);
            } else if (myminor != dminor) {
                std::cerr << "Warning: Minor versions of merged messages do not agree:" << myminor << " != " << dminor << std::endl;
            }
        }
        m->ParseFromString(message->SerializeAsString());
        return m;
    }

    A4Message A4Message::operator+(const A4Message & _m2) const {
        using google::protobuf::Descriptor;
        using google::protobuf::DescriptorPool;
        using google::protobuf::Message;

        // Find out which descriptor to use. Prefer dynamic descriptors
        // since they are probably contain all fields.
        const Descriptor * d;
        shared<DescriptorPool> sp;
        const DescriptorPool * p;
        if (_dynamic_descriptor) {
            d = _dynamic_descriptor;
            sp = _pool;
            p = sp.get(); 
        } else if (_m2._dynamic_descriptor) {
            d = _m2._dynamic_descriptor;
            sp = _m2._pool;
            p = sp.get();
        } else {
            d = _descriptor;
            p = DescriptorPool::generated_pool();
        }

        shared<Message> m1 = this->as_dynamic_message(d, p);
        shared<Message> m2 = _m2.as_dynamic_message(d, p);
        shared<Message> merged(m1->New());

        for (int i = 0; i < d->field_count(); i++) {
            MetadataMergeOptions merge_opts = d->field(i)->options().GetExtension(merge);

            DynamicField f1(*m1, d->field(i));
            DynamicField f2(*m2, d->field(i));
            DynamicField fm(*merged, d->field(i));

            switch(merge_opts) {
                case MERGE_BLOCK_IF_DIFFERENT:
                    if(!(f1 == f2)) throw a4::Fatal("Trying to merge metadata objects with different entries in ", f1.name());
                    break;
                case MERGE_ADD:
                    add_fields(f1, f2, fm);
                    break;
                case MERGE_MULTIPLY:
                    multiply_fields(f1, f2, fm);
                    break;
                case MERGE_UNION:
                    append_fields(f1, f2, fm, true);
                    break;
                case MERGE_APPEND:
                    append_fields(f1, f2, fm, false);
                    break;
                default:
                    throw a4::Fatal("Unknown merge strategy: ", merge_opts, ". Recompilation should fix it.");
            }
        }
        return A4Message(merged, d, d, sp);
    }
    
    std::string A4Message::field_as_string(const std::string & field_name) {
        const FieldDescriptor* fd = _descriptor->FindFieldByName(field_name);
        DynamicField f(*message, fd);
        if (f.repeated()) {
            std::stringstream ss;
            for (int i = 0; i < f.size(); i++) ss << f.value(i).str();
            return ss.str();
        } else {
            return f.value().str();
        }
    }


};};
