#ifndef _A4_MESSAGE_H_
#define _A4_MESSAGE_H_

#include <string>
#include <vector>
#include <deque>

#include <a4/a4io.h>

// used internally
namespace google{ namespace protobuf{
    class Message;
};};

namespace a4{ namespace io{
    /// Wrapped message returned from the InputStream
    class A4Message {
        public:
            /// Construct A4Message that signifies end of stream or stream error
            A4Message(bool error=false) { if (error) class_id = 1; else class_id = 0; message.reset(); };
            /// Construct normal A4Message with class_id and protobuf Message
            A4Message(uint32_t cls, shared<Message> msg) : class_id(cls), message(msg) {};
            /// Class ID of the message read
            uint32_t class_id;
            /// shared protobuf message 
            shared<Message> message;

            /// true if an error occurred
            bool error() const {return class_id == 1; };
            /// true if the stream has terminated correctly
            bool end() const {return class_id == 0; };
            /// true if the message pointer is None (end, unknown or no metadata)
            bool null() const {return message.get() == NULL; };
            /// this object can be used in if() expressions, it will be true if it contains a message
            operator bool() const { return !null(); }
            bool operator!() const { return null(); }
            /// Check if the class ID matches.
            /// example: if (result.is<TestEvent>())
            template <class T>
            bool is() const { return T::kCLASSIDFieldNumber == class_id; };
            /// Check if the class ID matches and return the message, otherwise NULL.
            /// example: auto event = result.as<MyEvent>()
            template <class T>
            shared<T> as() const {
                if (not is<T>()) return shared<T>(); 
                else return static_shared_cast<T>(message);
            }
    };
};};

#endif
