#include <iostream>
#include <a4/application.h>
#include "Event.pb.h"

using namespace std;
using namespace a4::process;
using namespace a4::example::root;

class MyProcessor : public ProcessorOf<Event, Metadata> {
  public:
    virtual bool process(const Event & event) {
        cout << event.run_number() << " MD: " << metadata().total_events() << endl;
    }
};

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
