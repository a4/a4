#ifndef _A4_APPDRIVERS_H_
#define _A4_APPDRIVERS_H_

#include <a4/driver.h>

namespace a4{ 
    namespace io{
        class A4Input;
        class A4Output;
    }
    namespace process{

        class SimpleCommandLineDriver : public Driver {
            public:
                SimpleCommandLineDriver(Configuration *);
                int main(int argc, const char * argv[]);
            protected:
                static void simple_thread(SimpleCommandLineDriver* self, Processor * p);
                Processor * new_initialized_processor();
                Configuration * configuration;
                std::string metakey;
                shared<a4::io::A4Input> in;
                shared<a4::io::A4Output> out, res;
                int threads;
        };

        template <class MyConfiguration, class MyDriver=SimpleCommandLineDriver>
        int a4_main_configuration(int argc, const char * argv[]) {
            a4::Fatal::enable_throw_on_segfault();
            shared<MyDriver> driver(new MyDriver(argc, argv, new MyConfiguration()));
            return driver->main(argc, argv);
        };

        template <class MyProcessor, class MyDriver=SimpleCommandLineDriver>
        int a4_main_process(const int &argc, const char * argv[]) {
            a4::Fatal::enable_throw_on_segfault();
            unique<Configuration> p(new ConfigurationOf<MyProcessor>());
            unique<MyDriver> driver(new MyDriver(p.get()));
            return driver->main(argc, argv);
        };

    }; // namespace a4::process
}; // namespace a4

#endif
