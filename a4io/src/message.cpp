#include <set>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/message.h>

#include <A4.pb.h>

#include "dynamic_message.h"
#include "proto_class_pool.h"

using google::protobuf::DynamicMessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::Message;

namespace a4{ namespace io{

    void UnreadMessage::invalidate_stream() {
        auto coded_in = _coded_in.lock();
        if (coded_in) {
            //std::cerr << "Invalidating stream by reading " << _size << std::endl;
            if (not coded_in->ReadString(&_bytes, _size)) {
                std::cerr << "Invalidating stream failed!" << std::endl;
                coded_in->PushLimit(0);                
            }
            _coded_in.reset();
            _size = 0;
        }
    }

    /// Construct A4Message that signifies end of stream or stream error
    A4Message::A4Message() : _class_id(0), _descriptor(NULL) 
    { 
        _unread_message.reset();
        _pool.reset();
    }

    /// Constructs an unread A4Message connected to a ProtoClassPool
    A4Message::A4Message(uint32_t class_id, shared<UnreadMessage> umsg, shared<ProtoClassPool> pool) :
        _unread_message(umsg),
        _class_id(class_id),
        _descriptor(pool->descriptor(class_id)),
        _pool(pool)
    {
    }

    /// Constructs an read A4Message connected to a ProtoClassPool
    A4Message::A4Message(uint32_t class_id, shared<Message> msg, shared<ProtoClassPool> pool) :
        _unread_message(new UnreadMessage(msg)),
        _class_id(class_id),
        _descriptor(pool->descriptor(class_id)),
        _pool(pool)
    { 
    }

    /// Construct an A4Message from a compiled-in protobuf Message
    A4Message::A4Message(shared<google::protobuf::Message> msg,
                       bool metadata) 
        : _unread_message(new UnreadMessage(msg)),
          _class_id(metadata ? NO_CLASS_ID_METADATA : NO_CLASS_ID),
          _descriptor(msg->GetDescriptor())
    {
        _pool.reset();
    }

    /// Construct an A4Message from a compiled-in protobuf Message
    A4Message::A4Message(const google::protobuf::Message& msg,
                        bool metadata) 
        : _unread_message(new UnreadMessage(shared<google::protobuf::Message>(msg.New()))),
          _class_id(metadata ? NO_CLASS_ID_METADATA : NO_CLASS_ID),
          _descriptor(msg.GetDescriptor())
    {
        _unread_message->_message->CopyFrom(msg);
        _pool.reset();
    }

    /// This copy constructor is assumed to be cheap
    A4Message::A4Message(const A4Message& m) : 
        _unread_message(m._unread_message),
        _class_id(m._class_id),
        _descriptor(m._descriptor),
        _pool(m._pool)
    {
    }

    A4Message::~A4Message() {
        _unread_message.reset();
        _pool.reset();
    }

    const google::protobuf::Message* A4Message::message() const {
        if (_unread_message) {
            if (_unread_message->_message) {
                

            } else if (_unread_message->_coded_in.lock()) {
                _unread_message->_message = _pool->parse_message(
                        _class_id, 
                        _unread_message->_coded_in, 
                        _unread_message->_size);
                _unread_message->_coded_in.reset();
                _unread_message->_size = 0;

            } else {
                _unread_message->_message = _pool->parse_message(
                        _class_id, 
                        _unread_message->_bytes);
                _unread_message->_bytes = "";
            }
            if (not _unread_message->_message)
                FATAL("Unable to parse message!");
            return _unread_message->_message.get();
        }
        return NULL;
    }

    const std::string A4Message::bytes() const {
        if (not _unread_message) FATAL("Trying to get bytes of empty message");
        if (_unread_message->_message) {
            return _unread_message->_message->SerializeAsString();
        }
        _unread_message->invalidate_stream();
        return _unread_message->_bytes;
    }

    void A4Message::version_check(const A4Message &m2) const {
        if (descriptor()->full_name() != m2.descriptor()->full_name()) {
            FATAL("Typenames of objects to merge do not agree: ", descriptor()->full_name(), " != ", m2.descriptor()->full_name());
        }
        const Descriptor* d1 = _pool->dynamic_descriptor(_class_id);
        if (not d1) d1 = _descriptor;

        const Descriptor* d2 = m2._pool->dynamic_descriptor(m2._class_id);
        if (not d2) d2 = m2._descriptor;
         
        if (d1 == d2) return;

        // Do version checking if the dynamic descriptors are different
        std::string mymajor = d1->options().GetExtension(major_version);
        std::string myminor = d1->options().GetExtension(minor_version);
        std::string dmajor = d2->options().GetExtension(major_version);
        std::string dminor = d2->options().GetExtension(minor_version);

        if (mymajor != dmajor) {
            FATAL("Major versions of objects to merge do not agree:", mymajor, " != ", dmajor);
        } else if (myminor != dminor) {
            WARNING("Minor versions of merged messages do not agree:",
                    myminor, " != ", dminor);
        }
    }

    A4Message A4Message::operator+(const A4Message& m2_) const {
        // Find out which descriptor to use. Prefer dynamic descriptors
        // since they are probably contain all fields.
        version_check(m2_);

        const Descriptor* dd = dynamic_descriptor();
        const Descriptor* d = dd;
        if (not d) d = _descriptor;

        A4Message res(_class_id, _pool->get_new_message(d), _pool);

        // Force every message to be constructed by the same descriptor
        A4Message m1, m2;
        m1 = m2 = res;
        if (m2_._descriptor == d) {
            m1 = *this;
        } else {
            m1._unread_message->_message.reset(res.message()->New());
            m1._unread_message->_message->ParseFromString(bytes());
        }

        if (m2_._descriptor == d) {
            m2 = m2_;
        } else {
            m2._unread_message->_message.reset(res.message()->New());
            m2._unread_message->_message->ParseFromString(m2_.bytes());
        }

        for (int i = 0; i < d->field_count(); i++) {
            MetadataMergeOptions merge_opts = d->field(i)->options().GetExtension(merge);

            ConstDynamicField f1(*m1.message(), d->field(i));
            ConstDynamicField f2(*m2.message(), d->field(i));
            DynamicField fm(*res._unread_message->_message, d->field(i));

            switch(merge_opts) {
                case MERGE_BLOCK_IF_DIFFERENT:
                    if(!(f1 == f2)) FATAL("Trying to merge metadata objects with different entries in ", f1.name());
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
                    FATAL("Unknown merge strategy: ", merge_opts, ". Recompilation should fix it.");
            }
        }
        return res;
    }

    A4Message& A4Message::operator+=(const A4Message& m2_) {
        // Find out which descriptor to use. Prefer dynamic descriptors
        // since they are probably contain all fields.
        version_check(m2_);

        const Descriptor* dd = dynamic_descriptor();
        const Descriptor* d = dd;
        if (not d) d = _descriptor;

        A4Message m1, m2;
        m1 = m2 = A4Message(_class_id, _pool->get_new_message(d), _pool);

        if (_descriptor == d) {
            m1 = *this;

        } else {
            m1._unread_message->_message.reset(m1._unread_message->_message->New());
            m1._unread_message->_message->ParseFromString(bytes());
        }

        if (m2_._descriptor == d) {
            m2 = m2_;
        } else {
            m2._unread_message->_message.reset(m2._unread_message->_message->New());
            m2._unread_message->_message->ParseFromString(m2_.bytes());
        }

        message(); // force message to be read
        m1.message();
        m2.message();

        for (int i = 0; i < d->field_count(); i++) {
            MetadataMergeOptions merge_opts = d->field(i)->options().GetExtension(merge);

            DynamicField f1(*m1._unread_message->_message, d->field(i));
            ConstDynamicField f2(*m2._unread_message->_message, d->field(i));

            switch(merge_opts) {
                case MERGE_BLOCK_IF_DIFFERENT:
                    if(!(f1 == f2)) {
                        DEBUG("this=", message()->DebugString());
                        DEBUG("msg1=", m1._unread_message->_message->DebugString());
                        DEBUG("msg2=", m2._unread_message->_message->DebugString());
                        throw a4::Fatal("Trying to merge metadata objects with different entries in ", f1.name(), ":", f1.value().str(), " != ", f2.value().str());
                    }
                    // NOOP!
                    //f1.set(f1.value());
                    break;
                case MERGE_ADD:
                    inplace_add_fields(f1, f2);
                    break;
                case MERGE_MULTIPLY:
                    inplace_multiply_fields(f1, f2);
                    break;
                case MERGE_UNION:
                    inplace_append_fields(f1, f2);
                    break;
                case MERGE_APPEND:
                    inplace_append_fields(f1, f2);
                    break;
                case MERGE_DROP:
                    break;
                default:
                    throw a4::Fatal("Unknown merge strategy: ", merge_opts, ". Recompilation should fix it.");
            }
        }
        return *this;
    }
    
    void A4Message::unionize() {
        assert(_unread_message->_message);
        Message& m = *_unread_message->_message;
        const auto* refl = m.GetReflection();
        //const auto* desc = m.GetDescriptor();
        
        std::vector<const FieldDescriptor*> filled_fields;
        refl->ListFields(m, &filled_fields);
        
        foreach (const auto* field, filled_fields) {
            MetadataMergeOptions merge_opts = field->options().GetExtension(merge);
            
            DynamicField thisfield(m, field);
            
            switch (merge_opts) {
                case MERGE_UNION:
                {
                    assert(thisfield.repeated());
                    //if (thisfield.message()) FATAL("Can't merge union field with complex type");
                    std::set<FieldContent> unioned;
                    for (int i = 0; i < thisfield.size(); i++)
                        unioned.insert(thisfield.value(i));
                    thisfield.clear();
                    foreach (auto& value, unioned)
                        thisfield.add(value);
                    break;
                }
                default:
                    break;
            }
        }
    }
    
    std::string A4Message::field_as_string(const std::string& field_name) {
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

    std::string A4Message::assert_field_is_single_value(const std::string& field_name) {
        assert(descriptor() == message()->GetDescriptor());
        const FieldDescriptor* fd = descriptor()->FindFieldByName(field_name);
        if (!fd) {
            const std::string& classname = message()->GetDescriptor()->full_name();
            FATAL(classname, " has no member ", field_name, " necessary for metadata merging or splitting!");
        }
        if (fd->is_repeated() && (message()->GetReflection()->FieldSize(*message(), fd)) > 1) {
            FATAL(fd->full_name(), " has already multiple ", field_name, " entries - cannot achieve desired granularity!");
        }
        return field_as_string(field_name);
    }
    
    const google::protobuf::Descriptor* A4Message::dynamic_descriptor() const {;
        return _pool->dynamic_descriptor(_class_id);
    }

};};
