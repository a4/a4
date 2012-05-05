#ifndef _A4_MESSAGE_H_
#define _A4_MESSAGE_H_

#include <string>

#include <a4/types.h>

// used internally
namespace google {
namespace protobuf {
    class Descriptor;
    class DescriptorPool;
    class Message;
    namespace io {
        class CodedInputStream;
    }
}
}

namespace a4 {
namespace io {
    class ProtoClassPool;
    class ConstDynamicField;
    using google::protobuf::Message;

    /// Wrapped message returned from the InputStream
    static const uint32_t NO_CLASS_ID = ((1LL<<32) - 2);
    static const uint32_t NO_CLASS_ID_METADATA = ((1LL<<32) - 1);

    class A4Message {
        public:
            /// Construct an A4Message of given size going to be read from the given stream
            A4Message(uint32_t class_id, size_t size, weak_shared<google::protobuf::io::CodedInputStream> coded_in, shared<ProtoClassPool> pool);

            /// Constructs an read A4Message connected to a ProtoClassPool
            A4Message(uint32_t class_id, shared<Message> msg, shared<ProtoClassPool> pool);

            /// Construct an A4Message from a compiled-in protobuf Message
            explicit A4Message(shared<Message> msg, bool metadata=true);

            /// Construct an A4Message from a compiled-in protobuf Message
            explicit A4Message(const Message& msg, bool metadata=true);

            /// Only explicit copying allowed
            explicit A4Message(const A4Message& m);
            ~A4Message();
            
            /// Get to protobuf Message of the A4Message, parsing if necessary
            const Message* message() const;

            /// Get the serialized bytes of the message
            const std::string& bytes() const;

            /// Get the number serialized bytes of the message
            size_t bytesize() const;
            
            /// Pointer to the descriptor of that message, used for quick type checks.
            const google::protobuf::Descriptor* descriptor() const { return _descriptor; }

            /// Pointer to the dynamic descriptor of that message if possible
            const google::protobuf::Descriptor* dynamic_descriptor() const;

            /// Class ID on the wire
            uint32_t class_id() const { return _class_id; }

            /// Returns true if this is a metadata message
            bool metadata() const { return class_id() % 2 == 1; }

            /// Quick check if the message is of that class
            /// example: if (result.is<TestEvent>())
            template <class T>
            bool is() const { return T::descriptor() == descriptor(); }

            /// Checked Cast of the message to the given class.
            /// Returns null if the cast fails.
            /// example: auto event = result.as<MyEvent>()
            template <class T>
            const T* as() const {
                assert_valid();
                if (not is<T>())  return NULL;
                return static_cast<const T*>(message());
            }
            
            template <class T>
            T* as_mutable() {
                assert_valid();
                if (not is<T>()) return NULL;
                message();
                _valid_bytes = false; // invalidate bytes since message may be changed
                return static_cast<T*>(_message.get());
            }

            /// Merge two messages that support it via the "merge" field extension
            A4Message& operator+=(const A4Message& rhs);
           
            /// Enforce the UNION merge option - removes duplicates 
            void unionize();

            /// Invalidate any stream that is saved internally and force bytes to be read
            void invalidate_stream() const;

            /// Return a field of this message in string representation
            std::string field_as_string(const std::string& field_name) const;
            std::string assert_field_is_single_value(const std::string& field_name) const;
            /// Returns true if the field specified by `field_name` can be merged in the two messages
            bool check_key_mergable(const A4Message& rhs, const std::string& field_name) const;

            bool assert_valid() const;
            
            shared<ConstDynamicField> dynamic_field(const std::string& field_name) const;
        private:

            void version_check(const A4Message &m2) const;

            /// Class ID on the wire (can be different for different headers)
            uint32_t _class_id;

            /// Pointer to the descriptor of that message, used for quick type checks.
            const google::protobuf::Descriptor* _descriptor;

            /// Shared pointer to the pool of that message, so that it does not disappear on us.
            shared<ProtoClassPool> _pool;

            /// Size of the message in bytes
            size_t _size;

            /// True if the _bytes fields contains valid data. Necessary since the empty string
            /// is always a valid protobuf message.
            mutable bool _valid_bytes;

            /// Coded input stream that this message can be read from
            mutable weak_shared<google::protobuf::io::CodedInputStream> _coded_in;

            /// Encoded Message
            mutable std::string _bytes;

            /// The Message itself
            mutable shared<Message> _message;

            /// True if the input stream has been read
            mutable bool _instream_read;

            friend class OutputStream;
    };
};};

#endif
