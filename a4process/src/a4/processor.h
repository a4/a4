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
        template <class This, class T, class... TArgs> struct _test_process_as<This, T, TArgs...> { 
            static bool process(This* that, const std::string &n, shared<Storable> s) { 
                shared<T> t = dynamic_pointer_cast<T>(s);
                if (t) {
                    that->process(n, t);
                    return true;
                } else return _test_process_as<This, TArgs...>::process(that, n, s);
            }
        };
        template <class This> struct _test_process_as<This> { static bool process(This* that, const std::string &n, shared<Storable> s) { return false; }; };

        using a4::io::A4Message;

        class Driver;

        class Processor {
            public:
                Processor() : auto_metadata(true) {};
                virtual ~Processor() {};
                /// Override this to process raw A4 Messages without type checking
                virtual void process_message(const A4Message) = 0;
                /// This function is called if new metadata is available
                virtual void process_new_metadata() {};
                bool write(const google::protobuf::Message& m) { if (_outstream) return _outstream->write(m); else return false; };
                void metadata(shared<const google::protobuf::Message> m) { out_metadata = m; }

                /// The whole analysis is rerun with the prefix "channel/<name>/" and in that run this function always returns true
                bool channel(const char * name) {
                    rerun_channels.insert(name);
                    return rerun_channels_current == name;
                }
                /// The whole analysis is rerun with the prefix "syst/<name>/" and in that run this function always returns true
                bool systematic(const char * name) {
                    rerun_systematics.insert(name);
                    return rerun_systematics_current == name;
                }
                /// From now all histograms are also saved with the prefix "channel/<name>/"
                // void now_channel(const char * name);
                /// (unimplemented) From now on, all histos are saved under prefix "syst/<name>/" and scale <scale>
                // void scale_systematic(const char * c, double scale) { throw a4::Fatal("Not Implemented"); return false; };

            protected:
                shared<a4::io::InputStream> _instream;
                shared<a4::io::OutputStream> _outstream;
                shared<ObjectBackStore> _backstore;
                ObjectStore S;
                A4Message metadata_message;
                shared<const google::protobuf::Message> out_metadata;
                bool auto_metadata;

                std::set<const char *> rerun_channels;
                const char * rerun_channels_current;
                std::set<const char *> rerun_systematics;
                const char * rerun_systematics_current;

                friend class a4::process::Driver;
        };

        class Configuration {
            public: 
                virtual ~Configuration() {};
                /// Override this to add options to the command line and configuration file
                virtual po::options_description get_options() { return po::options_description(); };
                /// Override this to do further processing of the options from the command line or config file
                virtual void read_arguments(po::variables_map &arguments) {};

                virtual void setup_processor(Processor &g) {};
                virtual Processor * new_processor() = 0;
        };

        template<class ProtoMessage, class ProtoMetaData = a4::io::NoProtoClass>
        class ProcessorOf : public Processor {
            public:
                ProcessorOf() { a4::io::RegisterClass<ProtoMessage> _e; a4::io::RegisterClass<ProtoMetaData> _m; };
                /// Override this to proces only your requested messages
                virtual void process(const ProtoMessage &) = 0;
                void process_message(const A4Message msg) {
                    if (!msg) throw a4::Fatal("No message!"); // TODO: Should not be fatal
                    ProtoMessage * pmsg = msg.as<ProtoMessage>().get();
                    if (!pmsg) throw a4::Fatal("Unexpected Message type: ", typeid(*msg.message.get()), " (Expected: ", typeid(ProtoMessage), ")");
                    process(*pmsg);
                };
                const ProtoMetaData & metadata() {
                    const A4Message msg = metadata_message;
                    if (!msg) throw a4::Fatal("No metadata at this time!"); // TODO: Should not be fatal
                    ProtoMetaData * meta = msg.as<ProtoMetaData>().get();
                    if (!meta) throw a4::Fatal("Unexpected Metadata type: ", typeid(*msg.message.get()), " (Expected: ", typeid(ProtoMetaData), ")");
                    return *meta;
                };
            protected:
                friend class a4::process::Driver;
        };

        template<class This, class ProtoMetaData = a4::io::NoProtoClass, class... Args>
        class ResultsProcessor : public Processor {
            public:
                ResultsProcessor() { a4::io::RegisterClass<ProtoMetaData> _m; have_name = false; };

                // Generic storable processing
                virtual void process(const std::string &, Storable &) {};

                void process_message(const A4Message msg) {
                    shared<Storable> next = _next_storable(msg);
                    if (next) {
                        if(!_test_process_as<This, Args...>::process((This*)this, next_name, next)) {
                            process(next_name, *next);
                        }
                    }
                };

                shared<Storable> _next_storable(const A4Message msg);

                const ProtoMetaData & metadata() {
                    const A4Message msg = metadata_message;
                    if (!msg) throw a4::Fatal("No metadata at this time!"); // TODO: Should not be fatal
                    ProtoMetaData * meta = msg.as<ProtoMetaData>().get();
                    if (!meta) throw a4::Fatal("Unexpected Metadata type: ", typeid(*msg.message.get()), " (Expected: ", typeid(ProtoMetaData), ")");
                    return *meta;
                };
            protected:
                std::string next_name;
                bool have_name;
                virtual const int metadata_class_id() const { return ProtoMetaData::kCLASSIDFieldNumber; };
                friend class a4::process::Driver;
        };


        template<class MyProcessor>
        class ConfigurationOf : public Configuration {
            public:
                /// Override this to setup your thread-safe Processor!
                virtual void setup_processor(MyProcessor &g) {};

                virtual void setup_processor(Processor &g) { setup_processor(dynamic_cast<MyProcessor&>(g)); };
                virtual Processor * new_processor() { return new MyProcessor(); };
        };
    };
};

#endif
