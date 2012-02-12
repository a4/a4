#include <iostream>

#include <a4/application.h>
#include <a4/process/A4Key.pb.h>
#include <a4/io/A4Stream.pb.h>

using namespace a4::process;
using namespace a4::io;

class ToyHist : public StorableAs<ToyHist, TestHisto> {
    public:
        void constructor(const float& start, int A, int B) {
            j = start;
            pb.reset(new TestHisto());
        }
        void to_pb(bool blank_pb) { pb->set_data(j); };
        void from_pb() { j = pb->data(); };
        ToyHist& operator+=(const ToyHist& other) { j += other.j; return *this; };
        ToyHist& operator*=(const double&) { return *this; };
        void add(float s) { j += s; };
        float j;
};

//class MyProcessor : public ProcessorOf<TestHisto, TestHistoMetaData> {
class MyProcessor : public ProcessorOf<TestEvent, TestMetaData> {
    public:
        virtual void process(const TestHisto&);
        virtual void process(const TestEvent&);
};

void MyProcessor::process(const TestHisto& event) {
    DEBUG("Bin number: ", event.bin_number());
    write(event);
    S.T<ToyHist>("hugo")(4.2, 0, 1).add(event.bin_number());
}

void MyProcessor::process(const TestEvent& event) {
    write(event);
    S.T<ToyHist>("hugo")(4.2, 0, 1).add(event.event_number());
}

int main(int argc, const char* argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
}
