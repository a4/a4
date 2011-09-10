#include <iostream>

#include "a4/output_stream.h"
#include "a4/proto/io/A4Stream.pb.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    std::string fn;
    if (argc == 1) {
        fn = "test_nomd_bw.a4";
    } else if (argc == 2) {
        fn = argv[1];
    } else assert(argc <= 2);

    uint32_t clsid = TestEvent::kCLASSIDFieldNumber;
    uint32_t clsid_m = TestMetaData::kCLASSIDFieldNumber;
    A4OutputStream w(fn, "TestEvent", clsid, 0, false, true);

    const int N = 500;
    TestEvent e;
    for(int i = 0; i < N; i++) {
        e.set_event_number(1000 + i);
        w.write(e);
    }
    for(int i = 0; i < N; i++) {
        e.set_event_number(2000 + i);
        w.write(e);
    }
}
