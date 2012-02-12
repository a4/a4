#include <iostream>

#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/results_processor.h>
#include <a4/object_store.h>

using namespace std;
using namespace boost::filesystem;
using namespace a4::process;
using namespace a4::io;
using namespace a4::hist;

class A4MergeProcessor : public ResultsProcessor<A4MergeProcessor, a4::io::NoProtoClass, H1, H2, H3, Cutflow> {
  public:
    void process(std::string name, shared<Storable> s) {
        if (S.get_slow<Storable>(name)) {
            (*S.get_slow<Storable>(name)) += *s; 
        } else {
            S.set_slow(name, s); 
        }
    }
};

int main(int argc, const char* argv[]) {
    return a4_main_process<A4MergeProcessor>(argc, argv);
};
