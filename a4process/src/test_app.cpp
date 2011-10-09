#include <iostream>

#include <a4/application.h>
#include <a4/proto/process/A4Key.pb.h>

using std::cout; using std::cerr; using std::endl;

using namespace a4::process;

class MyProcessor : public ProcessorOf<TestHisto, TestHistoMetaData> {
    public:
        virtual void process(const TestHisto &);
};

void MyProcessor::process(const TestHisto & event) {
    cout << event.bin_number() << endl;
}

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
