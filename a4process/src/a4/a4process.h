#ifndef _A4PROCESS_H_
#define _A4PROCESS_H_

#include <boost/program_options.hpp>

#include <a4/a4io.h>
#include <a4/message.h>
#include <a4/register.h>

namespace po = ::boost::program_options;

namespace a4{
    /// Processing utilities for A4.
    ///
    /// To implement an Analysis that processes MyEvents, and has MyMetaData,
    /// derive a class from ProcessorOf<MyEvent, MyMetaData>.
    /// If you want to process more than one type of Events, or a4 files 
    /// containing histograms, derive directly from Processor.
    ///
    /// If your analysis needs configuration or setup (command-line options, external
    /// smearing classes, ...) derive a configuration class from Configuration
    /// 
    namespace process{
        using a4::io::A4Message;

        class Driver;

        class Processor {
            public:
                /// Override this to proces raw A4 Messages without type checking
                virtual bool process_message(const A4Message) = 0;
                /// This function is called if new metadata is available
                virtual bool new_metadata() {};
                const A4Message metadata_message();
            protected:
                shared<a4::io::A4InputStream> _instream;
                friend class a4::process::Driver;
        };

        class Configuration {
            public:
                /// Override this to add options to the command line and configuration file
                virtual po::options_description get_options() { return po::options_description(); };
                /// Override this to do further processing of the options from the command line or config file
                virtual bool read_arguments(po::variables_map &arguments) {};

                virtual bool setup_processor(Processor &g) { return true; };
                virtual Processor * new_processor() = 0;
        };

        template<class ProtoMessage, class ProtoMetaData = a4::io::NoProtoClass>
        class ProcessorOf : public Processor {
            public:
                ProcessorOf() { a4::io::RegisterClassID<ProtoMessage> _e; a4::io::RegisterClassID<ProtoMetaData> _m; };
                /// Override this to proces only your requested messages
                virtual bool process(const ProtoMessage &) = 0;
                bool process_message(const A4Message msg) { return process(*msg.as<ProtoMessage>()); };
                const ProtoMetaData & metadata() {
                    const A4Message msg = metadata_message();
                    return *msg.as<ProtoMetaData>().get();
                };
        };

        template<class MyProcessor>
        class ConfigurationOf : public Configuration {
            public:
                /// Override this to setup your thread-safe Processor!
                virtual bool setup_processor(MyProcessor &g) { return true; };

                virtual bool setup_processor(Processor &g) { return setup_processor(dynamic_cast<MyProcessor&>(g)); };
                virtual Processor * new_processor() { return new MyProcessor(); };
        };
    };
};

#endif
