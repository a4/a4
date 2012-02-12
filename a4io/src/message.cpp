#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/message.h>

#include <A4.pb.h>

#include "dynamic_message.h"

using google::protobuf::DynamicMessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::Message;

namespace a4{ namespace io{

    A4Message::~A4Message() {
        _message.reset();
        _factory.reset();
        _pool.reset();
    }

    void A4Message::version_check(const A4Message &m2) const {
        if (descriptor()->full_name() != m2.descriptor()->full_name()) {
            throw a4::Fatal("Typenames of objects to merge do not agree: ", descriptor()->full_name(), " != ", m2.descriptor()->full_name());
        }
        const Descriptor * d1;
        if (_dynamic_descriptor) {
            d1 = _dynamic_descriptor;
        } else if (_pool) {
            d1 = _pool->FindMessageTypeByName(descriptor()->full_name());
        } else {
            d1 = _descriptor;
        }
        
        const Descriptor * d2;
        if (_dynamic_descriptor) {
            d2 = m2._dynamic_descriptor;
        } else if (m2._pool) {
            d2 = m2._pool->FindMessageTypeByName(m2.descriptor()->full_name());
        } else {
            d2 = m2._descriptor;
        }
        if (d1 == d2) return;

        // Do version checking if the dynamic descriptors are different
        std::string mymajor = d1->options().GetExtension(major_version);
        std::string myminor = d1->options().GetExtension(minor_version);
        std::string dmajor = d2->options().GetExtension(major_version);
        std::string dminor = d2->options().GetExtension(minor_version);

        if (mymajor != dmajor) {
            throw a4::Fatal("Major versions of objects to merge do not agree:", mymajor, " != ", dmajor);
        } else if (myminor != dminor) {
            WARNING("Minor versions of merged messages do not agree:",
                    myminor, " != ", dminor);
        }
    }

    A4Message A4Message::operator+(const A4Message & m2_) const {
        // Find out which descriptor to use. Prefer dynamic descriptors
        // since they are probably contain all fields.
        version_check(m2_);

        const Descriptor * d;
        const Descriptor * dd;
        if (m2_._dynamic_descriptor) {
            dd = d = m2_._dynamic_descriptor;
        } else if (m2_._pool) {
            dd = d = m2_._pool->FindMessageTypeByName(m2_.descriptor()->full_name());
        } else {
            d = m2_._descriptor;
            dd = NULL;
        }

        // Prepare dynamic messages
        A4Message res, m1, m2;
        res = m2_;
        res._descriptor = d;
        res._dynamic_descriptor = dd;
        m1 = m2 = res;

        if (m2._factory) {
            res._message.reset(m2._factory->GetPrototype(d)->New());
        } else {
            res._message.reset(m2.message()->New());
        }

        if (_descriptor == d) {
            m1 = *this;
        } else {
            m1._message.reset(res.message()->New());
            m1._message->ParseFromString(_message->SerializeAsString());
        }

        if (m2_._descriptor == d) {
            m2 = m2_;
        } else {
            m2._message.reset(res.message()->New());
            m2._message->ParseFromString(m2_.message()->SerializeAsString());
        }

        for (int i = 0; i < d->field_count(); i++) {
            MetadataMergeOptions merge_opts = d->field(i)->options().GetExtension(merge);

            ConstDynamicField f1(*m1.message(), d->field(i));
            ConstDynamicField f2(*m2.message(), d->field(i));
            DynamicField fm(*res._message, d->field(i));

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
        return res;
    }
    
    std::string A4Message::field_as_string(const std::string & field_name) {
        assert(descriptor() == message()->GetDescriptor());
        const FieldDescriptor* fd = descriptor()->FindFieldByName(field_name);
        ConstDynamicField f(*message(), fd);
        if (f.repeated()) {
            std::stringstream ss;
            for (int i = 0; i < f.size(); i++)
                ss << f.value(i).str();
            return ss.str();
        } else {
            return f.value().str();
        }
    }

    std::string A4Message::assert_field_is_single_value(const std::string & field_name) {
        assert(descriptor() == message()->GetDescriptor());
        const FieldDescriptor* fd = descriptor()->FindFieldByName(field_name);
        if (!fd) {
            const std::string & classname = message()->GetDescriptor()->full_name();
            throw a4::Fatal(classname, " has no member ", field_name, " necessary for metadata merging or splitting!");
        }
        if (fd->is_repeated() && (message()->GetReflection()->FieldSize(*message(), fd)) > 1) {
            throw a4::Fatal(fd->full_name(), " has already multiple ", field_name, " entries - cannot achieve desired granularity!");
        }
        return field_as_string(field_name);
    }

};};
