#ifndef _A4_MESSAGE_H_
#define _A4_MESSAGE_H_

#include <string>

#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/types.h>
#include <a4/register.h>

// used internally
namespace google {
namespace protobuf {
    class Descriptor;
    class DescriptorPool;
};};

namespace a4 {
namespace io {
    class ProtoClassPool;

    /// Wrapped message returned from the InputStream
    static const uint32_t NO_CLASS_ID = ((1L<<32) - 2);
    static const uint32_t NO_CLASS_ID_METADATA = ((1L<<32) - 1);

    class UnreadMessage {
        public:
            UnreadMessage(size_t size, weak_shared<google::protobuf::io::CodedInputStream> coded_in)
                : _size(size), _coded_in(coded_in), _bytes()
            {
                _message.reset();
            }
            UnreadMessage(shared<google::protobuf::Message> message)
                : _size(0), _coded_in(), _bytes(), _message(message)
            {
            }
            UnreadMessage(const UnreadMessage&) = delete;
            void invalidate_stream();
            size_t _size;
            weak_shared<google::protobuf::io::CodedInputStream> _coded_in;
            std::string _bytes;
            /// If _message is set, we have parsed the message and _unread_message must be unset.
            mutable shared<google::protobuf::Message> _message;
    };
    
    class A4Message {
        public:
            /// Construct A4Message that signifies end of stream or stream error
            A4Message();
            /// Constructs an unread A4Message connected to a ProtoClassPool
            A4Message(uint32_t class_id, shared<UnreadMessage> umsg, shared<ProtoClassPool> pool);
            /// Constructs an read A4Message connected to a ProtoClassPool
            A4Message(uint32_t class_id, shared<Message> msg, shared<ProtoClassPool> pool);
            /// Construct an A4Message from a compiled-in protobuf Message
            explicit A4Message(shared<google::protobuf::Message> msg, bool metadata=true);
            /// Construct an A4Message from a compiled-in protobuf Message
            explicit A4Message(const google::protobuf::Message& msg, bool metadata=true);
            /// This copy constructor is assumed to be cheap
            A4Message(const A4Message& m); 
            ~A4Message();
            
            const google::protobuf::Message* message() const;
            const std::string bytes() const;
            
            /// Pointer to the descriptor of that message, used for quick type checks.
            const google::protobuf::Descriptor* descriptor() const { return _descriptor; }

            /// Pointer to the dynamic descriptor of that message if possible
            const google::protobuf::Descriptor* dynamic_descriptor() const;

            /// Class ID on the wire
            uint32_t class_id() const { return _class_id; }

            /// Returns true if this is a metadata message
            bool metadata() const { return class_id() % 2 == 1; }

            /// true if the message pointer is None (end, unknown or no metadata)
            bool null() const { return (not _unread_message); }

            /// this object can be used in if() expressions, it will be true if it contains a message
            operator bool() const { return !null(); }
            bool operator!() const { return null(); }

            /// Quick check if the message is of that class
            /// example: if (result.is<TestEvent>())
            template <class T>
            bool is() const { return T::descriptor() == descriptor(); }

            /// Checked Cast of the message to the given class.
            /// Returns null if the cast fails.
            /// example: auto event = result.as<MyEvent>()
            template <class T>
            const T* as() const {
                if (not is<T>())  return NULL;
                return static_cast<const T*>(message());
            }
            
            template <class T>
            T* as_mutable() {
                if (not is<T>()) return NULL;
                if (not _unread_message) return NULL;
                if (not _unread_message->_message)
                    message();
                return static_cast<T*>(_unread_message->_message.get());
            }

            /// Merge two messages that support it via the "merge" field extension
            A4Message operator+(const A4Message& rhs) const;
            A4Message& operator+=(const A4Message& rhs);
           
            /// Enforce the UNION merge option - removes duplicates 
            void unionize();

            /// Return a field of this message in string representation
            std::string field_as_string(const std::string& field_name);
            std::string assert_field_is_single_value(const std::string& field_name);
            
        private:
            void version_check(const A4Message &m2) const;

            /// If _unread_message is set, the message has not yet been parsed.
            mutable shared<UnreadMessage> _unread_message;

            /// Class ID on the wire (can be different for different headers)
            uint32_t _class_id;
            /// Pointer to the descriptor of that message, used for quick type checks.
            const google::protobuf::Descriptor* _descriptor;
            /// Shared pointer to the pool of that message, so that it does not disappear on us.
            shared<ProtoClassPool> _pool;
    };
};};

#endif
