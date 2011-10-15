#ifndef _A4_STREAM_H_
#define _A4_STREAM_H_

#include <boost/function.hpp>

#include <google/protobuf/message.h>

#include <a4/a4io.h>
#include <a4/types.h>

#include <a4/proto/io/A4.pb.h>

namespace google{ namespace protobuf{ namespace io{ class CodedInputStream; };};};

namespace a4{ namespace io{

    namespace internal {
        typedef boost::function<shared<Message> (google::protobuf::io::CodedInputStream*)> from_stream_func;
        typedef struct classreg_struct {
            classreg_struct() : descriptor(NULL), from_stream(NULL) {};
            const google::protobuf::Descriptor* descriptor;
            from_stream_func from_stream;
        } classreg;
    
        classreg map_class(std::string name, uint32_t class_id=0, classreg=classreg(), bool warn=true);

        template <typename ProtoClass>
        shared<Message> from_stream(google::protobuf::io::CodedInputStream * instr) {
            shared<ProtoClass> msg(new ProtoClass());
            msg->ParseFromCodedStream(instr);
            return msg;
        }

        template <typename ProtoClass>
        classreg reg_protoclass() {
            classreg r;
            r.descriptor = ProtoClass::descriptor();
            r.from_stream = from_stream<ProtoClass>;
            return map_class(r.descriptor->full_name(), 0, r);
        }
    }

    template <typename ProtoClass>
    class RegisterClass {
        public:
            static internal::classreg registration;
            // force static to be initialized
            virtual internal::classreg _get_registration() { return registration; }
    };

    template <typename ProtoClass>
    uint32_t _fixed_class_id() {
        return ProtoClass::descriptor()->options().GetExtension(fixed_class_id);
    }

    template <typename ProtoClass>
    internal::classreg RegisterClass<ProtoClass>::registration = internal::reg_protoclass<ProtoClass>();

    /// Class indicating that no MetaData is being used
    /// Has to implement all pure virtual methods of Message :P
    class NoProtoClass : public Message {
        public:
            NoProtoClass() {};
            virtual Message* New() const {return NULL;};
            virtual int GetCachedSize() const {return 0;};
            virtual google::protobuf::Metadata GetMetadata() const { return google::protobuf::Metadata(); };
            static const int kCLASSIDFieldNumber = 0;
            void ParseFromCodedStream(google::protobuf::io::CodedInputStream *) {};
    };

};};

#define A4RegisterClass(X) template class a4::io::RegisterClassID<X>;

#endif
