#include <iostream>
#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/root/atlas/ntup_smwz/Event.pb.h>

using namespace std;
using namespace a4::process;
using namespace a4::hist;
using namespace a4::root::atlas::ntup_smwz;

class MyProcessor : public ProcessorOf<Event> {
  public:
    virtual void process(const Event & event) {
        cout << event.event_number() << endl;
        S.T<H1>("test")(100,0,10).fill(2.1);
        write(event);
    }
};

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
