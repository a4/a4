#ifndef _A4_APPDRIVERS_H_
#define _A4_APPDRIVERS_H_

#include <a4/driver.h>
#include <a4/output_stream.h>

namespace a4 {
    namespace io {
        class A4Input;
        class A4Output;
    }
    namespace process {
        
        class ProcessStats;

        class SimpleCommandLineDriver : public Driver {
            public:
                SimpleCommandLineDriver(Configuration *);
                int main(int argc, const char* argv[]);
            protected:
                static void simple_thread(SimpleCommandLineDriver*, Processor *, int, ProcessStats&, std::exception_ptr&);
                Processor* new_initialized_processor();
                Configuration* configuration;
                std::string metakey, split_metakey;
                shared<a4::io::A4Input> in;
                shared<a4::io::A4Output> out, res;
                int threads;
                std::string _compression_string;
                a4::io::OutputStream::CompressionType _compression_type;
                int _compression_level;
        };

        template <class MyConfiguration, class MyDriver=SimpleCommandLineDriver>
        int a4_main_configuration(int argc, const char* argv[]) {
            a4::io::set_program_name(argv[0]);
            UNIQUE<Configuration> p(new MyConfiguration);
            UNIQUE<MyDriver> driver(new MyDriver(p.get()));
            return driver->main(argc, argv);
        }

        template <class MyProcessor, class MyDriver=SimpleCommandLineDriver>
        int a4_main_process(const int& argc, const char* argv[]) {
            a4::io::set_program_name(argv[0]);
            UNIQUE<Configuration> p(new ConfigurationOf<MyProcessor>());
            UNIQUE<MyDriver> driver(new MyDriver(p.get()));
            return driver->main(argc, argv);
        }

    } // namespace a4::process
} // namespace a4

#endif
