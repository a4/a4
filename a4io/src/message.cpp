#include <set>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/message.h>

#include <a4/io/A4.pb.h>

#include <a4/dynamic_message.h>
#include "proto_class_pool.h"

using google::protobuf::DynamicMessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::Message;


namespace a4{ namespace io{

    /// A4Message can reference a stream that has not yet been read.
    /// This function forces the read to happen and saves the result in _bytes
    void A4Message::invalidate_stream() const {
        if (not _instream_read) {
            auto coded_in = _coded_in.lock();
            _bytes.reserve(_size);
            if (not coded_in->ReadString(&_bytes, _size)) {
                FATAL("Invalidating stream failed!");
                coded_in->PushLimit(0);
                _valid_bytes = false;
            } else {
                _valid_bytes = true;
                _instream_read = true;
            }
            _coded_in.reset();
            coded_in.reset();
        }
    }
    
    /// Explicitly (deep-)copy an A4Message from a reference. 
    /// If the input A4Message has a protobuf _message, it is serialized to the
    /// _bytes of this message. We could also use protobuf message copying, but
    /// this would mean an additional protobuf CopyFrom if the message is
    /// eventually written to file.
    /// If you want to modify the message after copying, use the protobuf
    /// message() directly and use pb->New() and pb->CopyFrom().
    A4Message::A4Message(const A4Message& m)
            : _class_id(m._class_id), _descriptor(m._descriptor),
            _pool(m._pool), _size(0), _valid_bytes(), _coded_in(),
            _bytes(), _message(), _instream_read(true)
    {
        // Make sure that the read stream that the original message references
        // is read. After this call m._bytes or m._message (or both) are valid.

        if (not m._coded_in.expired()) {
            m.invalidate_stream();
        }

        _descriptor = m._descriptor;
        _size = m._size;
        _valid_bytes = m._valid_bytes;

        if (_valid_bytes) {
            DEBUG("_valid_bytes true");
            _bytes = m._bytes;

        } else if (m._message and not _valid_bytes) {
            DEBUG("_valid_bytes decoded..");

            //_bytes = m._message->SerializeAsString();
            _valid_bytes = true;

            // TODO(pwaller): If the following code is missing, a4atlas tests
            //                 will crash.
            
            // If the original message is not present in serialized form
            // (_valid_bytes) then save the serialization
            // _message.reset(m._message->New());
            // _message->CopyFrom(*m._message);
        } else {
            FATAL_ASSERT(false, "codepath shouldn't be reached.");
        }

        assert_valid();
    }
    
    /// Constructs an unread A4Message connected to a ProtoClassPool
    A4Message::A4Message(uint32_t class_id, size_t size, weak_shared<google::protobuf::io::CodedInputStream> coded_in, shared<ProtoClassPool> pool)
            : _class_id(class_id), _descriptor(pool->descriptor(class_id)),
            _pool(pool), _size(size), _valid_bytes(false), _coded_in(coded_in), _bytes(), _message(), _instream_read(false)
    {
        assert_valid();
    }

    /// Constructs an read A4Message connected to a ProtoClassPool
    A4Message::A4Message(uint32_t class_id, shared<Message> msg, shared<ProtoClassPool> pool)
            : _class_id(class_id), _descriptor(msg->GetDescriptor()),
              _pool(pool), _size(0), _valid_bytes(false), _coded_in(), _bytes(), _message(msg), _instream_read(true)
    { 
        assert_valid();
    }

    /// Construct an A4Message from a compiled-in protobuf Message
    A4Message::A4Message(shared<google::protobuf::Message> msg,
                       bool metadata)
            : _class_id(metadata ? NO_CLASS_ID_METADATA : NO_CLASS_ID),
              _descriptor(msg->GetDescriptor()), _pool(), _size(0),
              _valid_bytes(false), _coded_in(), _instream_read(true)
    {
        assert_valid();
    }

    /// Construct an A4Message from a compiled-in protobuf Message
    A4Message::A4Message(const google::protobuf::Message& msg,
                        bool metadata) 
            : _class_id(metadata ? NO_CLASS_ID_METADATA : NO_CLASS_ID),
              _descriptor(msg.GetDescriptor()), _pool(), _size(0),
              _valid_bytes(false), _coded_in(), _bytes(), _message(msg.New()), _instream_read(true)
    {
        _message->CopyFrom(msg);
        assert_valid();
    }

    A4Message::~A4Message() {
    }

    const google::protobuf::Message* A4Message::message() const {
        assert_valid();
        if (_message) return _message.get();

        if (_valid_bytes) {
            _message = _pool->parse_message(_class_id, _bytes);
        } else if (not _coded_in.expired()) {
            auto coded_in = _coded_in.lock();
            assert(coded_in);
            _message = _pool->parse_message(_class_id, coded_in, _size);
            _instream_read = true;
            coded_in.reset();
            _coded_in.reset();
        } else {
            FATAL("Called message() on empty A4Message!");
        }
        
        if (not _message)
            FATAL("Unable to parse message!");

        assert_valid();
        return _message.get();
    }

    const std::string& A4Message::bytes() const {
        if (not _coded_in.expired()) {
            invalidate_stream();
        }
        if (not _valid_bytes) {
            _bytes = _message->SerializeAsString();
            _valid_bytes = true;
        }
        assert_valid();
        return _bytes;
    }
    
    size_t A4Message::bytesize() const {
        if (not _coded_in.expired()) {
            return _size;
        } else if (_valid_bytes) {
            return _bytes.size();
        } else {
            return bytes().size();
        }
    }

    void A4Message::self_version_check() const {
        assert_valid();

        // OK if we have no dynamic descriptor
        if (not _pool) return;

        // OK if the descriptor is the dynamically loaded one
        auto d1 = descriptor();
        auto d2 = dynamic_descriptor();
        if (d1 == d2) return;

        // Fail if the full name is different (should not happen)
        if (d1->full_name() != d2->full_name()) {
            TERMINATE("Compiled-in (", d1->full_name(), ") and on-disk (", d2->full_name() , ") typenames of an object do not agree!");
        }

        // Do version checking if the descriptors are different
        std::string mymajor = d1->options().GetExtension(major_version);
        std::string myminor = d1->options().GetExtension(minor_version);
        std::string dmajor = d2->options().GetExtension(major_version);
        std::string dminor = d2->options().GetExtension(minor_version);

        if (mymajor != dmajor) {
            TERMINATE("Major versions of compiled-in and on-disk '", d1->full_name(), "' objects do not agree:", mymajor, " != ", dmajor);
        } else if (myminor != dminor) {
            WARNING("Minor versions of compiled-in and on-disk '", d1->full_name(), "' objects do not agree:", mymajor, " != ", dmajor);
        }
    }

    void A4Message::version_check(const A4Message &m2) const {
        if (descriptor() == m2.descriptor()) return;
    
        if (descriptor()->full_name() != m2.descriptor()->full_name()) {
            FATAL("Typenames of objects to merge do not agree: ", descriptor()->full_name(), " != ", m2.descriptor()->full_name());
        }
        const Descriptor* d1 = dynamic_descriptor();
        if (not d1) d1 = _descriptor;

        const Descriptor* d2 = m2.dynamic_descriptor();
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
        assert_valid();
    }

    A4Message& A4Message::operator+=(const A4Message& m2_) {
        // First, check self-compatibility of this and the next message
        // This ensures that they are compatible with the compiled-in
        // version of the message, if any
        self_version_check();
        m2_.self_version_check();

        // Now, check compatibility with each other
        version_check(m2_);

        // Use the compiled in descriptor preferentially
        // since it is probably newer (Change from previous behavior)
        const Descriptor* d = descriptor();

        // In case that the second message has a different descriptor
        // we have to re-read it with that one
        // This should not happen with compiled-in descriptors, but
        // may/will happen with dynamically loaded ones over file boundaries
        const A4Message* m2 = &m2_;
        shared<A4Message> m2tmp; // this will delete m2 at the end of the function. Do not put inside if statement!
        if (m2_._descriptor != d) {
            auto new_msg = _pool->get_new_message(d);
            assert(new_msg);
            assert(new_msg->GetDescriptor() == d);
            m2tmp.reset(new A4Message(_class_id, new_msg, _pool));
            m2tmp->_message->ParseFromString(m2_.bytes());
            m2 = m2tmp.get();
        }

        // force messages to be read
        message(); 
        m2->message();

        // invalidate the bytes since we are going to update this message
        _valid_bytes = false;

        for (int i = 0; i < d->field_count(); i++) {
            MetadataMergeOptions merge_opts = d->field(i)->options().GetExtension(merge);

            DynamicField f1(*_message, d->field(i));
            ConstDynamicField f2(*m2->_message, d->field(i));
            
            if (!f1.present() && !f2.present())
                continue;

            switch(merge_opts) {
                case MERGE_BLOCK_IF_DIFFERENT:
                    if(!(f1 == f2)) {
                        //DEBUG("this=", message()->DebugString());
                        //DEBUG("msg2=", m2->_message->DebugString());
                        TERMINATE("Trying to merge metadata objects with different entries in ", f1.name(), ":", f1.value().str(), " != ", f2.value().str());
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
                    TERMINATE("Unknown merge strategy: ", merge_opts, ". Recompilation should fix it.");
            }
        }
        assert_valid();
        return *this;
    }
    
    // For metadata merging, take the union of MERGE_UNION fields.
    void A4Message::unionize() {
        assert_valid();
        if (not _message) return;
        Message& m = *_message;
        _valid_bytes = false;
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
        assert_valid();
    }
    
    std::string A4Message::field_as_string(const std::string& field_name) const {
        assert(descriptor() == message()->GetDescriptor());
        assert_valid();
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
    
    bool A4Message::check_key_mergable(const A4Message& rhs, const std::string& field_name) const {
        assert(descriptor() == message()->GetDescriptor());
        assert_valid();
        
        auto lhs_field = dynamic_field(field_name),
             rhs_field = rhs.dynamic_field(field_name);
        
        if (lhs_field->present() != rhs_field->present())
            FATAL("Merge key is missing in one metadata but present in another");
        
        //if (lhs_fd != rhs_fd) {
            //FATAL("TODO(pwaller): Replace this assertion with a compatibility check");
        //}
        MetadataMergeOptions merge_opt = lhs_field->merge_option();
        if (merge_opt != rhs_field->merge_option())
            FATAL("Trying to merge messages with incompatible merge options");
        
        if (lhs_field->repeated() != rhs_field->repeated() or
            lhs_field->cpp_type() != rhs_field->cpp_type())
            FATAL("Attempting to merge incompatible fields");
        
        switch (merge_opt) {
            case MERGE_UNION:
                if (!lhs_field->repeated())
                    FATAL("MERGE_UNION on non-repeated field");
            case MERGE_BLOCK_IF_DIFFERENT:
                break;
            case MERGE_ADD:
            case MERGE_MULTIPLY:
            case MERGE_APPEND:
            case MERGE_DROP:
                FATAL("Merge is keyed on ", lhs_field->name(), " which has merge == ", MetadataMergeOptions_Name(merge_opt));
        }
        
        if (lhs_field->repeated()) {
            if (merge_opt == MERGE_UNION) {
                // This is the only merge type that makes sense for repeated values
                
                if (rhs_field->size() != 1)
                    FATAL("Metadata being merged already has multiple keys.");
                
                // Check that the last one is equal to this one.
                return lhs_field->value(lhs_field->size()-1) == rhs_field->value(0);
                
            } else {
                FATAL("Tried to merge keying on repeated field with non MERGE_UNION: ",
                      lhs_field->name(), " has merge == ", MetadataMergeOptions_Name(merge_opt));
            }
            FATAL("Bug. Please report this.");
        } else {
            return lhs_field->value() == rhs_field->value();
        }
        
        return false;
    }

    std::string A4Message::assert_field_is_single_value(const std::string& field_name) const {
        assert(descriptor() == message()->GetDescriptor());
        assert_valid();
        const FieldDescriptor* fd = descriptor()->FindFieldByName(field_name);
        if (!fd) {
            const std::string& classname = message()->GetDescriptor()->full_name();
            TERMINATE(classname, " has no member ", field_name, " necessary for metadata merging or splitting!");
        }
        if (fd->is_repeated() && (message()->GetReflection()->FieldSize(*message(), fd)) > 1) {
            TERMINATE(fd->full_name(), " has already multiple ", field_name, " entries - cannot achieve desired granularity!");
        }
        return field_as_string(field_name);
    }
    
    const google::protobuf::Descriptor* A4Message::dynamic_descriptor() const {
        assert(_pool);
        return _pool->dynamic_descriptor(_class_id);
    }
    
#ifdef ASSERT_MESSAGE
    bool A4Message::assert_valid() const {
        if (_message) {
            assert(_descriptor == _message->GetDescriptor());
            assert(_instream_read);
            return true;
        }
        if (_valid_bytes) {
            assert(_instream_read);
            return true;
        }
        if (not _coded_in.expired()) {
            assert(not _instream_read);
            return true;
        }
        assert(false);
    }
#endif
    
    shared<ConstDynamicField> A4Message::dynamic_field(const std::string& field_name) const {
        const FieldDescriptor* fd = descriptor()->FindFieldByName(field_name);
        FATAL_ASSERT(fd, "Field '", field_name, "' doesn't exist on ", descriptor()->full_name());
        return shared<ConstDynamicField>(new ConstDynamicField(*message(), fd));
    }

};};
