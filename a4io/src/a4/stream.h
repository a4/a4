#ifndef _A4_STREAM_H_
#define _A4_STREAM_H_

#define _FILE_OFFSET_BITS 64

#include <a4/a4io_config.h>
#include <google/protobuf/message.h>

namespace a4{ namespace io{

    using google::protobuf::Message;

    namespace internal {

        // Wizardry to get class from ID
        typedef shared<Message> (*from_stream_func)(google::protobuf::io::CodedInputStream *);
        from_stream_func all_class_ids(int, from_stream_func f = NULL);

        template <typename ProtoClass>
        shared<Message> from_stream(google::protobuf::io::CodedInputStream * instr) {
            auto msg = shared<Message>(new ProtoClass());
            msg->ParseFromCodedStream(instr);
            return msg;
        }

        template <typename ProtoClass>
        int reg_protoclass_id() {
            all_class_ids(ProtoClass::kCLASSIDFieldNumber, from_stream<ProtoClass>);
            return ProtoClass::kCLASSIDFieldNumber;
        }
    }

    template <typename ProtoClass>
    class UseClassID {
        public:
            static int class_id;
            virtual uint32_t get_class_id() { return class_id; } // forces class_id to be inited
    };

    template <typename ProtoClass>
    int UseClassID<ProtoClass>::class_id = internal::reg_protoclass_id<ProtoClass>();

};};

#endif
