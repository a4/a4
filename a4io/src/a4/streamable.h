#ifndef _A4_STREAMABLE_H_
#define _A4_STREAMABLE_H_

#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>

#include <boost/shared_ptr.hpp>


#include <iostream>

using google::protobuf::Message;
typedef boost::shared_ptr<Message> MessagePtr;

class Streamable {
    public:
        virtual ~Streamable() {};
        //Contract: implement this method:
        //static Streamable * from_message(google::protobuf::Message &) = 0;
        virtual MessagePtr get_message() const = 0;
        virtual uint32_t get_class_id() { return 0; };
};

// Wizardry to automatically generate a list of classes
// derived from StreamableTo<ProtoBufClass, MyClass>

typedef boost::shared_ptr<Streamable> (*from_msg_func)(google::protobuf::io::CodedInputStream *);


//extern std::map<int, from_msg_func> all_class_ids;
from_msg_func all_class_ids(int, from_msg_func f = NULL);

template <typename ProtoClass, typename MyStreamable>
boost::shared_ptr<Streamable> from_msg(google::protobuf::io::CodedInputStream * instr) {
    ProtoClass pc;
    pc.ParseFromCodedStream(instr);
    return boost::shared_ptr<Streamable>(MyStreamable::from_message(pc));
}

template <typename ProtoClass, typename MyStreamable>
int reg_class_id() {
    //all_class_ids[ProtoClass::kCLASSIDFieldNumber] = from_msg<ProtoClass, MyStreamable>;
    all_class_ids(ProtoClass::kCLASSIDFieldNumber, from_msg<ProtoClass, MyStreamable>);
    std::cerr << "registered class id " << ProtoClass::kCLASSIDFieldNumber << std::endl;
    return ProtoClass::kCLASSIDFieldNumber;
}

template <typename ProtoClass, typename MyStreamable>
class StreamableTo : virtual public Streamable {
    public:
        static int class_id;
        virtual uint32_t get_class_id() { return class_id; } // forces class_id to be inited

        virtual MyStreamable * clone_via_message() const {
            MessagePtr msg = get_message();
            return MyStreamable::from_message(*msg);
        };

        virtual MessagePtr get_message() const {
            if (!pb) pb.reset(new ProtoClass());
            to_pb();
            return pb;
        }

        static MyStreamable * from_message(Message &msg) {
            auto o = new MyStreamable();
            o->pb.reset(new ProtoClass());
            o->pb->CopyFrom(dynamic_cast<ProtoClass&>(msg));
            o->from_pb();
            return o;
        }

        mutable boost::shared_ptr<ProtoClass> pb;

    protected:
        virtual void to_pb() const {};
        virtual void from_pb() {};
};

template <typename ProtoClass, typename MyStreamable> 
int StreamableTo<ProtoClass, MyStreamable>::class_id = reg_class_id<ProtoClass, MyStreamable>();

#endif
