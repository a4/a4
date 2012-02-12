#ifndef _A4_MESSAGE_H_
#define _A4_MESSAGE_H_

#include <string>

#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>

#include <a4/types.h>

// used internally
namespace google {
namespace protobuf {
    class Descriptor;
    class DescriptorPool;
};};

namespace a4 {
namespace io {
    /// Wrapped message returned from the InputStream
    static const uint32_t NO_CLASS_ID = ((1L<<32) - 2);
    static const uint32_t NO_CLASS_ID_METADATA = ((1L<<32) - 1);
    
    class A4Message {
        public:
            /// Class-IDs for newly constructed messages

            /// Construct A4Message that signifies end of stream or stream error
            A4Message() : _class_id(0), _descriptor(NULL), 
                          _dynamic_descriptor(NULL) 
            { 
                _message.reset();
                _pool.reset();
                _factory.reset();
            }
            
            /// This copy constructor is assumed to be cheap
            A4Message(const A4Message &m) : 
                _message(m._message),
                _class_id(m._class_id),
                _descriptor(m._descriptor),
                _dynamic_descriptor(m._dynamic_descriptor),
                _pool(m._pool),
                _factory(m._factory) {}
                
            explicit A4Message(shared<google::protobuf::Message> msg, 
                               bool metadata=true) 
                : _message(msg),
                  _class_id(metadata ? NO_CLASS_ID_METADATA : NO_CLASS_ID),
                  _descriptor(msg->GetDescriptor()),
                  _dynamic_descriptor(NULL) 
            { 
                _pool.reset(); _factory.reset(); 
            }

            explicit A4Message(const google::protobuf::Message& msg,
                                bool metadata=true) 
                : _message(msg.New()),
                  _class_id(metadata ? NO_CLASS_ID_METADATA : NO_CLASS_ID),
                  _descriptor(_message->GetDescriptor()),
                  _dynamic_descriptor(NULL) 
            {
                _message->CopyFrom(msg);
            }
            ~A4Message();

            /// Construct normal A4Message with message, (static) descriptor, 
            /// dynamic descriptor and  class_id and protobuf Message
            A4Message(uint32_t class_id, 
                      shared<google::protobuf::Message> msg,
                      const google::protobuf::Descriptor* d,
                      const google::protobuf::Descriptor* dd,
                      shared<google::protobuf::DescriptorPool> pool,
                      shared<google::protobuf::DynamicMessageFactory> factory) 
                : _message(msg), _class_id(class_id), _descriptor(d), 
                  _dynamic_descriptor(dd), _pool(pool), _factory(factory) 
            {
                assert(_descriptor == msg->GetDescriptor());
            }

            /// Shared protobuf message 
            shared<google::protobuf::Message> _message;
            
            const google::protobuf::Message* message() const { return _message.get(); }
            
            /// Pointer to the descriptor of that message, used for quick type checks.
            const google::protobuf::Descriptor* descriptor() const { return _descriptor; }
            /// Class ID on the wire
            uint32_t class_id() const { return _class_id; }
            /// Returns true if this is a metadata message
            bool metadata() const { return class_id() % 2 == 1; }

            /// true if the message pointer is None (end, unknown or no metadata)
            bool null() const { return _message.get() == NULL; }

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
            shared<T> as() const {
                if (not is<T>()) return shared<T>();
                if (!_message)
                    message();
                return static_pointer_cast<T>(_message);
            }

            /// Merge two messages that support it via the "merge" field extension
            A4Message operator+(const A4Message& rhs) const;

            /// Return a field of this message in string representation
            std::string field_as_string(const std::string& field_name);
            std::string assert_field_is_single_value(const std::string& field_name);
            
        private:
            void version_check(const A4Message &m2) const;

            /// Class ID on the wire
            uint32_t _class_id;
            /// Pointer to the descriptor of that message, used for quick type checks.
            const google::protobuf::Descriptor* _descriptor;
            /// Pointer to the dynamically loaded descriptor, used for merging
            const google::protobuf::Descriptor* _dynamic_descriptor;
            /// Shared pointer to the descriptor pool of that message, so that it does not disappear on us.
            shared<google::protobuf::DescriptorPool> _pool;
            /// Shared pointer to the factory that created this message
            shared<google::protobuf::DynamicMessageFactory> _factory;

    };
};};

#endif
