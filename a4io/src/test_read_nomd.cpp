#include <iostream>

#include "a4/input_stream.h"
#include "a4/proto/io/A4Stream.pb.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    std::string fn;
    if (argc == 1) {
        fn = "test.a4";
    } else if (argc == 2) {
        fn = argv[1];
    } else assert(argc <= 2);

    A4InputStream r(fn);

    int cnt = 0;
    while (r.is_good()) {
        A4Message rr = r.next();
        if (rr.class_id == TestEvent::kCLASSIDFieldNumber) {
            auto te = static_shared_cast<TestEvent>(rr.object);
            //std::cout << "TestEvent: " << te->event_number() << std::endl;
            cnt++;
        } else if (rr.error()) throw "up";
    }
    return 0;
}
