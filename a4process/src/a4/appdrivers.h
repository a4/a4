#ifndef _A4_APPDRIVERS_H_
#define _A4_APPDRIVERS_H_

#include <a4/driver.h>

namespace a4{ namespace process{

class SimpleCommandLineDriver {
    public:
        SimpleCommandLineDriver(Configuration *);
        int main(int argc, const char * argv[]);
    protected:
        Configuration * configuration;
};

template <class MyConfiguration, class MyDriver=SimpleCommandLineDriver>
int a4_main_configuration(int argc, const char * argv[]) {
    auto driver = shared<MyDriver>(new MyDriver(argc, argv, new MyConfiguration()));
    return driver->main(argc, argv);
};

template <class MyProcessor, class MyDriver=SimpleCommandLineDriver>
int a4_main_process(const int &argc, const char * argv[]) {
    auto driver = shared<MyDriver>(new MyDriver(new ConfigurationOf<MyProcessor>()));
    return driver->main(argc, argv);
};

};}; // namespace a4::process

#endif
