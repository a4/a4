#include <iostream>

#include <boost/program_options.hpp>
#include <a4/a4process.h>
#include <a4/appdrivers.h>
#include <a4/proto/io/A4Stream.pb.h>

using std::cout; using std::cerr; using std::endl;
using a4::io::TestEvent; using a4::io::TestMetaData;
using namespace a4::process;

class MyProcessor : public ProcessorOf<TestEvent, TestMetaData> {
    public:
        virtual bool process(const TestEvent & event) {
            cout << event.event_number() << endl;
        }
};


int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
}


