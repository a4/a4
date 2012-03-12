#ifndef _A4_STORABLE_H_
#define _A4_STORABLE_H_

#include <a4/a4io.h>
#include <a4/types.h>


namespace google {
namespace protobuf {
    class Descriptor;
    class Message;
}
}


namespace a4 {
namespace store {


    /// Storable objects are objects that can be stored in an ObjectStore.
    /// They should be derived from StorableAs instead of directly from Storable.
    /// This class is mainly for "dealers in Storables" (heh).
    class Storable {
        public:
            Storable() : _current_weight(1.0) {}
            /// Get a Protobuf message that contains the information about this object
            virtual shared<const google::protobuf::Message> as_message() = 0;
            /// Use this function if the Storable should copy the info from the message
            virtual void construct_from(const google::protobuf::Message&) = 0;
            /// Use this function of the Storable is allowed to keep the message
            virtual void construct_from(shared<google::protobuf::Message> m) = 0;
            /// Require that merging works. Must throw an exception if merge fails. (may be bad_cast)
            virtual Storable& operator+=(const Storable &other) = 0;
            // Trying C++0x move semantics...
            virtual Storable&& operator+(const Storable &other) {
                return std::move(this->clone_storable() += other);
            }
            /// Require that reweighting works.
            virtual Storable& operator*=(const double&) = 0;
            // Trying C++0x move semantics...
            virtual Storable&& operator*(const double& v) {
                return std::move(this->clone_storable() *= v);
            }
            /// Require that a current weight can be set
            void weight(const double& weight) { _current_weight = weight; }
            /// Cloneable
            virtual Storable&& clone_storable() = 0;
            virtual ~Storable() {}
        protected:
            double _current_weight;
    };

    namespace internal {

        typedef shared<Storable> (*from_message_func)(const google::protobuf::Message&);
        from_message_func as_storable(const google::protobuf::Descriptor*, from_message_func f = NULL);

        template <typename Class>
        shared<Storable> from_message(const google::protobuf::Message& msg) {
            return Class::from_message(msg);
        }

        template <typename Class, typename ProtoClass>
        int reg_storable() {
            as_storable(ProtoClass::descriptor(), from_message<Class>);
            return true;
        }
    }

    /// Derive from this class if you want to write a result-type
    /// (like a histogram) that can be stored in an A4 Stream.
    /// Your derived class has to have four methods:
    ///  * "constructor" (one or more, arbitrary arguments. See operator() ).
    ///  * MyClass& operator+=(const MyClass &other); // Merging
    ///  * to_pb(bool blank_pb) // write data to this->pb. 
    ///  * from_pb() // Read data from this->pb or initialize this->pb as data
    ///
    /// There are two ways of using this->pb: 
    ///   1) Use this->pb as primary datastore (you have to init it in your constructors)
    ///   2) Read from this->pb into your own data and this->pb.reset() 
    ///
    template <class This, class ProtoClass>
    class StorableAs : public Storable {
        public:
            StorableAs<This, ProtoClass>() : _initializations_remaining(1) {
                pb.reset(new ProtoClass());
            };

            // Override this to provide merging
            virtual This& operator+=(const This &other) = 0;
 
            This && clone() {
                This t;
                t.construct_from(*(this->as_message()));
                return std::move(t);
            }

            Storable && clone_storable() { return clone(); };

            Storable& operator+=(const Storable &other) {
                static_cast<This&>(*this) += dynamic_cast<const This&>(other);
                return *this;
            }

            virtual shared<const google::protobuf::Message> as_message() {
                if (!pb) {
                    pb.reset(new ProtoClass());
                    to_pb(true);
                } else {
                    to_pb(false);
                }
                return pb; 
            }

            virtual void construct_from(const google::protobuf::Message& msg) {
                if(_initialized()) FATAL("Object already constructed!");
                pb.reset(new ProtoClass()); 
                pb->CopyFrom(msg);
                from_pb();
            }

            virtual void construct_from(shared<google::protobuf::Message> msg) {
                if(_initialized()) FATAL("Object already constructed!");
                pb = dynamic_pointer_cast<ProtoClass>(msg);
                from_pb();
            }

            static shared<This> from_message(const google::protobuf::Message& msg) {
                shared<This> t(new This());
                t->construct_from(msg);
                return t;
            }

            static shared<This> from_message(shared<google::protobuf::Message> msg) {
                shared<This> t(new This());
                t->construct_from(msg);
                return t;
            }

            /// This constructor is needed to prevent GCC from complaining and
            /// possibly to work with compilers < GCC4.6
            template <typename... Args> This& operator()(const std::initializer_list<double>& bins, const char* label="") {
                if (_initializations_remaining != 0) {
                    _initializations_remaining--;
                    static_cast<This*>(this)->constructor(bins, label);
                }
                return *static_cast<This*>(this);
            }
            
            /// This constructor is needed so that you can give integer bin widths
            template <typename... Args> This& operator()(const std::initializer_list<int>& bins, const char* label="") {
                if (_initializations_remaining != 0) {
                    _initializations_remaining--;
                    std::vector<double> double_bins;
                    for(auto edge = bins.begin(); edge != bins.end(); edge++)
                        double_bins.push_back(*edge);
                    // = bins;
                    static_cast<This*>(this)->constructor(double_bins, label);
                }
                return *static_cast<This*>(this);
            }
            
            template <typename... Args> This& operator()(const Args&... args) {
                if (_initializations_remaining != 0) {
                    _initializations_remaining--;
                    static_cast<This*>(this)->constructor(args...);
                }
                return *static_cast<This*>(this);
            }

            void assert_initialized() {
                if (_initializations_remaining != 0) {
                    try {
                        std::cerr << as_message()->DebugString() << std::endl;
                    } catch (...) {}
                    
                    FATAL("Uninitialized Object used!"); 
                }
            }
            
        protected:
            // Override this!
            virtual void to_pb(bool blank_pb) {}
            // Override this!
            virtual void from_pb() {}

            shared<ProtoClass> pb;

            int _initializations_remaining;
            bool _initialized() const { return _initializations_remaining == 0; }
            static bool _registered;
            virtual bool get_registered() { return _registered; } // forces _registered to be inited
    };

    template <class This, class ProtoClass>
    bool StorableAs<This, ProtoClass>::_registered = internal::reg_storable<This, ProtoClass>();

    shared<Storable> message_to_storable(shared<const a4::io::A4Message> msg);

}
}

#endif
