#include <iostream>

#include "a4/input_stream.h"
#include "a4/io/A4Stream.pb.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    std::string fn;
    if (argc == 1) {
        fn = "test.a4";
    } else if (argc == 2) {
        fn = argv[1];
    } else assert(argc <= 2);

    InputStream r(fn);

    int cnt = 0;
    while (A4Message rr = r.next()) {
        if (rr.is<TestEvent>()) {
            shared<TestEvent> te = rr.as<TestEvent>();
            A4Message md = r.current_metadata();
            assert(!md.null());
            assert(te->event_number() / 1000 == md.as<TestMetaData>()->meta_data());
            //std::cout << "TestEvent: " << te->event_number() << std::endl;
            cnt++;
        }
    }
    if (r.error()) throw "up";
    return 0;
}
