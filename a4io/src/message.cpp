#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/message.h>

#include <A4.pb.h>

#include "dynamic_message.h"

using google::protobuf::DynamicMessageFactory;

namespace a4{ namespace io{

    A4Message::~A4Message() {
        std::cerr << "RESET MESSAGE" <<std::endl;
        message.reset();
        std::cerr << "RESET FACTORY" <<std::endl;
        _factory.reset();
        std::cerr << "RESET POOL" <<std::endl;
        _pool.reset();
    }

    A4Message A4Message::as_dynamic_message(const google::protobuf::Descriptor* d, shared<google::protobuf::DescriptorPool> p) const {
        if (d == descriptor()) return *this;

        shared<DynamicMessageFactory> _message_factory;

        if (_factory) _message_factory = _factory;
        else _message_factory.reset(new DynamicMessageFactory(p.get()));

        shared<Message> m(_message_factory->GetPrototype(d)->New());
        // Do version checking if the dynamic descriptors are different
        if (d != _dynamic_descriptor) {
            const google::protobuf::Descriptor* my_d = _dynamic_descriptor ? _dynamic_descriptor : descriptor();

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
        return A4Message(class_id(), m, d, d, p, _message_factory);
    }

    A4Message A4Message::operator+(const A4Message & _m2) const {
        using google::protobuf::Descriptor;
        using google::protobuf::DescriptorPool;
        using google::protobuf::Message;

        // Find out which descriptor to use. Prefer dynamic descriptors
        // since they are probably contain all fields.
        uint32_t clsid;
        const Descriptor * d;
        shared<DescriptorPool> sp;
        const DescriptorPool * p;

        A4Message am1, am2;
        shared<Message> m1, m2;
        if (_dynamic_descriptor) {
            clsid = class_id();
            d = _dynamic_descriptor;
            sp = _pool;
            p = sp.get();
            am1 = this->as_dynamic_message(d, sp); 
            am2 = _m2.as_dynamic_message(d, sp); 
        } else if (_m2._dynamic_descriptor) {
            clsid = _m2.class_id();
            d = _m2._dynamic_descriptor;
            sp = _m2._pool;
            p = sp.get();
            am1 = this->as_dynamic_message(d, sp); 
            am2 = _m2.as_dynamic_message(d, sp); 
        } else {
            clsid = class_id();
            d = descriptor();
            p = DescriptorPool::generated_pool();
            am1 = *this;
            am2 = _m2;
        }
        m1 = am1.message;
        m2 = am2.message;
        shared<Message> merged(m1->New());

        for (int i = 0; i < d->field_count(); i++) {
            MetadataMergeOptions merge_opts = d->field(i)->options().GetExtension(merge);

            DynamicField f1(*m1, d->field(i));
            DynamicField f2(*m2, d->field(i));
            DynamicField fm(*merged, d->field(i));

            switch(merge_opts) {
                case MERGE_BLOCK_IF_DIFFERENT:
                    if(!(f1 == f2)) throw a4::Fatal("Trying to merge metadata objects with different entries in ", f1.name());
                    fm.set(f1.value());
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
                case MERGE_DROP:
                    break;
                default:
                    throw a4::Fatal("Unknown merge strategy: ", merge_opts, ". Recompilation should fix it.");
            }
        }
        return A4Message(clsid, merged, d, d, sp, am1._factory);
    }
    
    std::string A4Message::field_as_string(const std::string & field_name) {
        const FieldDescriptor* fd = descriptor()->FindFieldByName(field_name);
        DynamicField f(*message, fd);
        if (f.repeated()) {
            std::stringstream ss;
            for (int i = 0; i < f.size(); i++) ss << f.value(i).str();
            return ss.str();
        } else {
            return f.value().str();
        }
    }

    std::string A4Message::assert_field_is_single_value(const std::string & field_name) {
        const FieldDescriptor* fd = descriptor()->FindFieldByName(field_name);
        if (!fd) {
            const std::string & classname = message->GetDescriptor()->full_name();
            throw a4::Fatal(classname, " has no member ", field_name, " necessary for metadata merging or splitting!");
        }
        if (fd->is_repeated() && (message->GetReflection()->FieldSize(*message, fd)) > 1) {
            throw a4::Fatal(fd->full_name(), " has already multiple ", field_name, " entries - cannot achieve desired granularity!");
        }
        return field_as_string(field_name);
    }

};};
