#include <iostream>

#include <a4/application.h>
#include <a4/proto/process/A4Key.pb.h>

using std::cout; using std::cerr; using std::endl;

using namespace a4::process;

class ToyHist : public StorableAs<ToyHist, TestHisto> {
    public:
        void constructor(const float & start, int A, int B) {
            j = start;
            pb.reset(new TestHisto());
        }
        void to_pb(bool blank_pb) { pb->set_data(j); };
        void from_pb() { j = pb->data(); };
        ToyHist & operator+=(const ToyHist &other) { j += other.j; return *this; };
        void add(float s) { j += s; };
        float j;
};

class MyProcessor : public ProcessorOf<TestHisto, TestHistoMetaData> {
    public:
        virtual void process(const TestHisto &);
};

void MyProcessor::process(const TestHisto & event) {
    cout << event.bin_number() << endl;
    write(event);
    S.T<ToyHist>("hugo")(4.2, 0, 1).add(event.bin_number());
}

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
