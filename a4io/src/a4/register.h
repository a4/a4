#ifndef _A4_STREAM_H_
#define _A4_STREAM_H_

#include <boost/function.hpp>

#include <google/protobuf/message.h>

#include <a4/a4io.h>

namespace google{ namespace protobuf{ namespace io{ class CodedInputStream; };};};

namespace a4{ namespace io{

    namespace internal {
        typedef struct classreg_struct {
            classreg_struct() : descriptor(NULL), from_stream(NULL) {};
            const google::protobuf::Descriptor* descriptor;
            boost::function<shared<Message> (google::protobuf::io::CodedInputStream*)> from_stream;
        } classreg;
    
        classreg map_class_by_name(std::string name, classreg=classreg(), bool warn=true);

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
            return map_class_by_name(r.descriptor->full_name(), r);
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
