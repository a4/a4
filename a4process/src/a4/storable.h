#ifndef _A4_STORABLE_H_
#define _A4_STORABLE_H_

#include <a4/a4io.h>
#include <a4/message.h>

namespace a4{ namespace process{

    /// Storable objects are objects that can be stored in an ObjectStore.
    /// They should be derived from StorableAs instead of directly from Storable.
    /// This class is mainly for "dealers in Storables" (heh).
    class Storable {
        public:
            Storable() {};
            /// Get a Protobuf message that contains the information about this object
            virtual shared<const google::protobuf::Message> as_message() = 0;
            /// Use this function if the Storable should copy the info from the message
            virtual void construct_from(const google::protobuf::Message&) = 0;
            /// Use this function of the Storable is allowed to keep the message
            virtual void construct_from(shared<google::protobuf::Message> m) = 0;
            /// Require that merging works. Must throw an exception if merge fails. (may be bad_cast)
            virtual Storable& operator+=(const Storable &other) = 0;
            // Trying C++0x move semantics...
            virtual Storable&& operator+(const Storable &other) { Storable && t = this->clone(); t += other; return t; };
            /// Cloneable
            virtual Storable&& clone() = 0;
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
 
            Storable && clone() { This t = *static_cast<This*>(this); return std::move(t); };
            Storable & operator+=(const Storable &other) {
                static_cast<This&>(*this) += dynamic_cast<const This&>(other);
                return *this;
            }
            virtual This & operator+=(const This &other) {};

            virtual shared<const google::protobuf::Message> as_message() {
                if (!pb) {
                    pb.reset(new ProtoClass());
                    to_pb(true);
                } else {
                    to_pb(false);
                }
                return pb; 
            };

            virtual void construct_from(const google::protobuf::Message& msg) {
                pb.reset(new ProtoClass()); 
                pb->CopyFrom(msg);
                from_pb();
            };

            virtual void construct_from(shared<google::protobuf::Message> msg) {
                pb = dynamic_shared_cast<ProtoClass>(msg);
                from_pb();
            };

            static shared<This> from_message(const google::protobuf::Message& msg) {
                shared<This> t(new This());
                t->construct_from(msg);
                return t;
            };

            static shared<This> from_message(shared<google::protobuf::Message> msg) {
                shared<This> t(new This());
                t->construct_from(msg);
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
            virtual void to_pb(bool blank_pb) {};
            virtual void from_pb() {};
            shared<ProtoClass> pb;
            bool _initialized;
    };

    template <class This, class ProtoClass>
    bool StorableAs<This, ProtoClass>::_registered = internal::reg_storable<This, ProtoClass>();

    shared<Storable> message_to_storable(a4::io::A4Message msg);

};};

#endif
