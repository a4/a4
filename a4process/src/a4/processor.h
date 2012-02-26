#ifndef _A4PROCESS_H_
#define _A4PROCESS_H_

#include <set>

#include <boost/program_options.hpp>

#include <a4/a4io.h>
#include <a4/types.h>
#include <a4/input_stream.h>
#include <a4/output_stream.h>
#include <a4/message.h>
#include <a4/register.h>
#include <a4/object_store.h>

namespace po = ::boost::program_options;

namespace a4{
    namespace process{
        //INTERNAL
        template <class This, typename... TArgs> struct _test_process_as;
        
        template <class This, class T, class... TArgs> 
        struct _test_process_as<This, T, TArgs...> { 
            static bool process(This* that, const std::string &n, shared<Storable> s) { 
                shared<T> t = dynamic_pointer_cast<T>(s);
                if (t) {
                    that->process(n, t);
                    return true;
                } else return _test_process_as<This, TArgs...>::process(that, n, s);
            }
        };
        
        template <class This> 
        struct _test_process_as<This> { 
            static bool process(This* that, const std::string& n, shared<Storable> s) { return false; }
        };

        using a4::io::A4Message;

        class Driver; 
        class Configuration;
        class OutputAdaptor {
            public:
                virtual void write(shared<const A4Message> m) = 0;
                virtual void metadata(shared<const A4Message> m) = 0;
                void write(const google::protobuf::Message& m) { write(shared<const A4Message>(new A4Message(m))); }
                void metadata(const google::protobuf::Message& m) { metadata(shared<const A4Message>(new A4Message(m))); }
        };

        class Processor {
            public:
                enum MetadataBehavior { AUTO, MANUAL_FORWARD, MANUAL_BACKWARD, DROP };
                MetadataBehavior get_metadata_behavior() { return metadata_behavior; }

                Processor() : my_configuration(NULL), skip_to_next_metadata(false), 
                              locked(false), metadata_behavior(AUTO) {}
                virtual ~Processor() {}

                /// This function is called at the start of a new metadata block
                /// In here you can return an alternate metadata message if
                /// if auto_metadata is true.
                virtual shared<A4Message> process_new_metadata() { return shared<A4Message>(); };

                /// Override this to process raw A4 Messages
                virtual void process_message(shared<const A4Message>) = 0;

                /// This function is called at the end of a metadata block
                virtual void process_end_metadata() {};

                /// Write a metadata message that (manual_metadata_forward ? starts : ends) a metadata block
                /// To use this method you have to disable automatic metadata writing.
                /// You also need to think about if you want to write your metadata before (manual_metadata_forward = true)
                /// or after (manual_metadata_forward = false) the events it refers to.
                void metadata_start_block(shared<A4Message> m) {
                    assert(metadata_behavior == MANUAL_FORWARD); 
                    _output_adaptor->metadata(m);
                }
                void metadata_start_block(const google::protobuf::Message& m) {
                    assert(metadata_behavior == MANUAL_FORWARD); 
                    _output_adaptor->metadata(m); 
                }
                void metadata_end_block(shared<A4Message> m) { 
                    assert(metadata_behavior == MANUAL_BACKWARD); 
                    _output_adaptor->metadata(m); 
                }
                void metadata_end_block(const google::protobuf::Message& m) {
                    assert(metadata_behavior == MANUAL_BACKWARD); 
                    _output_adaptor->metadata(m); 
                }

                /// Write a message to the output stream
                void write(shared<const A4Message> m) { _output_adaptor->write(m); }
                void write(const google::protobuf::Message& m) { _output_adaptor->write(m); }
                
                /// Write a message to the output stream at most once per event
                void skim(shared<const A4Message> m) {
                    if (not skim_written) write(m);
                    skim_written = true;
                }
                void skim(const google::protobuf::Message& m) {
                    if (not skim_written) write(m);
                    skim_written = true;
                }

                /// Call channel in process_message to rerun with the prefix "channel/<name>/".
                /// In that run this function always returns true.
                bool channel(const char* name) {
                    rerun_channels.insert(name);
                    if (rerun_channels_current == NULL) return false;
                    return strcmp(rerun_channels_current, name) == 0;
                }
                bool in_channel(const char* name) const {
                    if (rerun_channels_current == NULL) return false;
                    return strcmp(rerun_channels_current, name) == 0;
                }

                /// Call systematic in process_message to rerun with the prefix "syst/<name>/".
                /// In that run this function always returns true.
                bool systematic(const char* name) {
                    rerun_systematics.insert(name);
                    if (rerun_systematics_current == NULL) return false;
                    return strcmp(rerun_systematics_current, name) == 0;
                }
                bool in_systematic(const char* name) const {
                    if (rerun_systematics_current == NULL) return false;
                    return strcmp(rerun_systematics_current, name) == 0;
                }

                /// (Idea, unimplemented:) From now on, all histos are saved under prefix "syst/<name>/" and scale <scale>
                // void scale_systematic(const char* c, double scale) { FATAL("Not Implemented"); return false; };

                /// Access your own Configuration.
                /// WARNING: there is only one configuration per process, and it is shared by thread!
                /// Therefore, it is const. This may not prevent you from doing non-smart things with it.
                /// suggestion: Do a config = my<MyConfig>()," in your Processor.
                const Configuration* my_configuration;

                template<class T>
                const T* my() { return dynamic_cast<const T*>(my_configuration); }

                /// Set this flag to skip to the next metadata block
                bool skip_to_next_metadata;

            protected:
                /// In this store you can put named objects.
                /// It will be written and cleared at every metadata block boundary.
                ObjectStore S;

                /// This is the currently valid metadata message. If you manipulate it in AUTO mode
                /// in process_new_metadata the changes are written out.
                shared<const A4Message> metadata_message;

                /// Set the behaviour of metadata
                void set_metadata_behavior(MetadataBehavior m) { assert(!locked); metadata_behavior = m; }

                // Here follows internal stuff.
                std::set<const char *> rerun_channels;
                const char* rerun_channels_current;
                std::set<const char *> rerun_systematics;
                const char* rerun_systematics_current;

                OutputAdaptor* _output_adaptor;

                void lock_and_load() { locked = true; };
                bool skim_written;
                friend class a4::process::Driver;
                
            private:
                bool locked;
                MetadataBehavior metadata_behavior;
        };

        class Configuration {
            public: 
                virtual ~Configuration() {};
                /// Override this to add options to the command line and configuration file
                virtual void add_options(po::options_description_easy_init) {};
                /// Override this to do further processing of the options from the command line or config file
                virtual void read_arguments(po::variables_map &arguments) {};
                virtual void setup_processor(Processor &g) {};
                virtual Processor* new_processor() = 0;
        };

        template<class ProtoMessage, class ProtoMetaData = a4::io::NoProtoClass>
        class ProcessorOf : public Processor {
            public:
                ProcessorOf() {
                    a4::io::RegisterClass<ProtoMessage> _e;
                    a4::io::RegisterClass<ProtoMetaData> _m;
                }

                /// Override this to proces only your requested messages
                virtual void process(const ProtoMessage&) = 0;

                void process_message(shared<const A4Message> msg) {
                    if (!msg) FATAL("No message!"); // TODO: Should not be fatal
                    const ProtoMessage* pmsg = msg->as<ProtoMessage>();
                    if (!pmsg) FATAL("Unexpected Message type: ", typeid(*msg->message()), " (Expected: ", typeid(ProtoMessage), ")");
                    process(*pmsg);
                }

                const ProtoMetaData& metadata() {
                    if (!metadata_message) FATAL("No metadata at this time!"); // TODO: Should not be fatal
                    const ProtoMetaData* meta = metadata_message->as<ProtoMetaData>();
                    if (!meta) FATAL("Unexpected Metadata type: ", typeid(*metadata_message->message()), " (Expected: ", typeid(ProtoMetaData), ")");
                    return *meta;
                }

            protected:
                friend class a4::process::Driver;
        };

        template<class This, class ProtoMetaData = a4::io::NoProtoClass, class... Args>
        class ResultsProcessor : public Processor {
            public:
                ResultsProcessor() { 
                    a4::io::RegisterClass<ProtoMetaData> _m;
                    have_name = false; 
                }

                // Generic storable processing
                virtual void process_storable(const std::string&, shared<Storable>) {}

                void process_message(shared<const A4Message> msg) {
                    shared<Storable> next = _next_storable(msg);
                    if (next) {
                        if(!_test_process_as<This, Args...>::process((This*)this, next_name, next)) {
                            process_storable(next_name, next);
                        }
                    }
                }

                shared<Storable> _next_storable(shared<const A4Message> msg);

                const ProtoMetaData& metadata() {
                    shared<const A4Message> msg = metadata_message;
                    if (!msg) FATAL("No metadata at this time!"); // TODO: Should not be fatal
                    const ProtoMetaData* meta = msg->as<ProtoMetaData>();
                    if (!meta) FATAL("Unexpected Metadata type: ", typeid(*msg->message()), " (Expected: ", typeid(ProtoMetaData), ")");
                    return *meta;
                }
            
            protected:
                std::string next_name;
                bool have_name;
                friend class a4::process::Driver;
        };


        template<class MyProcessor>
        class ConfigurationOf : public Configuration {
            public:
                /// Override this to setup your thread-safe Processor!
                virtual void setup_processor(MyProcessor &g) {};

                virtual void setup_processor(Processor &g) { setup_processor(dynamic_cast<MyProcessor&>(g)); };
                virtual Processor* new_processor() { Processor* p = new MyProcessor(); p->my_configuration = this; return p;};
        };
    };
};

#endif
