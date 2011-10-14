#ifndef _A4_MESSAGE_H_
#define _A4_MESSAGE_H_

#include <string>
#include <vector>
#include <deque>

#include <google/protobuf/message.h>
#include <a4/a4io.h>

// used internally
namespace google{ namespace protobuf{
    class Message;
    class Descriptor;
};};

namespace a4{ namespace io{
    /// Wrapped message returned from the InputStream
    class A4Message {
        public:
            /// Construct A4Message that signifies end of stream or stream error
            A4Message() : descriptor(NULL) { message.reset(); };
            /// Construct normal A4Message with class_id and protobuf Message
            A4Message(const google::protobuf::Descriptor* d, shared<Message> msg) : message(msg), descriptor(d) {};

            /// Shared protobuf message 
            shared<Message> message;
            /// Pointer to the descriptor of that message, used for quick type checks.
            const google::protobuf::Descriptor* descriptor;

            /// true if the message pointer is None (end, unknown or no metadata)
            bool null() const {return message.get() == NULL; };

            /// this object can be used in if() expressions, it will be true if it contains a message
            operator bool() const { return !null(); }
            bool operator!() const { return null(); }

            /// Quick check if the message is of that class
            /// example: if (result.is<TestEvent>())
            template <class T>
            bool is() const { return T::descriptor() == descriptor; };

            /// Checked Cast of the message to the given class.
            /// Returns null if the cast fails.
            /// example: auto event = result.as<MyEvent>()
            template <class T>
            shared<T> as() const {
                if (not is<T>()) return shared<T>();
                return static_pointer_cast<T>(message);
            }
    };
};};

#endif
