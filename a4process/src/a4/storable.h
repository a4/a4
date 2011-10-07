#ifndef _A4_STORABLE_H_
#define _A4_STORABLE_H_

#include <a4/a4io.h>
#include <a4/message.h>

namespace a4{ namespace process{

    class Storable {
        public:

            virtual shared<google::protobuf::Message> as_message() = 0;
            virtual void set_message(const google::protobuf::Message&) = 0;
            virtual void set_message(shared<google::protobuf::Message> m) {set_message(m);};
            virtual ~Storable() {};

    };

    namespace internal {

        typedef shared<Storable> (*from_message_func)(const google::protobuf::Message&);
        from_message_func as_storable(int, from_message_func f = NULL);

        template <typename Class>
        shared<Storable> from_message(const google::protobuf::Message& msg) {
            return Class::from_message(msg);
        }

        template <typename Class, typename ProtoClass>
        int reg_storable() {
            as_storable(ProtoClass::kCLASSIDFieldNumber, from_message<Class>);
            return true;
        }
    }

    template <class This, class ProtoClass>
    class StorableAs : public Storable {
        public:
            StorableAs<This, ProtoClass>() : _initialized(false) {
                pb.reset(new ProtoClass());
            };

            virtual shared<google::protobuf::Message> as_message() { 
                to_pb(); 
                return pb; 
            };

            virtual void set_message(const google::protobuf::Message& msg) {
                pb.reset(new ProtoClass()); 
                pb->CopyFrom(msg);
                from_pb();
            };

            virtual void set_message(shared<google::protobuf::Message> msg) {
                pb = dynamic_shared_cast<ProtoClass>(msg);
                from_pb();
            };

            static shared<This> from_message(const google::protobuf::Message& msg) {
                shared<This> t(new This());
                t->set_message(msg);
                return t;
            };

            static shared<This> from_message(shared<google::protobuf::Message> msg) {
                shared<This> t(new This());
                t->set_message(msg);
                return t;
            };

            template <typename... Args> This& operator()(const Args&... args) {
                if (!_initialized) {
                    static_cast<This*>(this)->constructor(args...);
                    _initialized = true;
                }
                return *static_cast<This*>(this);
            }

        protected:
            static bool _registered;
            virtual bool get_registered() { return _registered; } // forces _registered to be inited
            virtual void to_pb() {};
            virtual void from_pb() {};
            shared<ProtoClass> pb;
            bool _initialized;
    };

    template <class This, class ProtoClass>
    bool StorableAs<This, ProtoClass>::_registered = internal::reg_storable<This, ProtoClass>();

    shared<Storable> message_to_storable(a4::io::A4Message msg);

};};

#endif
