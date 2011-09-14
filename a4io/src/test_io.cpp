#include <iostream>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output.h"
#include "a4/input.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    {
        uint32_t clsid = TestEvent::kCLASSIDFieldNumber;
        uint32_t clsid_m = TestMetaData::kCLASSIDFieldNumber;
        A4Output a4o("test_io.a4");

        auto w = a4o.get_stream("TestEvent", clsid, clsid_m);

        const int N = 1000;
        TestEvent e;
        for(int i = 0; i < N; i++) {
            e.set_event_number(i);
            w.write(e);
        }
        TestMetaData m;
        m.set_meta_data(N);
        w.metadata(m);
    }
    {
        A4Input in;
        in.add_file("test_io.a4");
        in.add_file("test_io.a4");
        while (shared<A4InputStream> & stream = in.get_stream()) {
            int cnt = 0;
            while (auto rr = r.next()) {
                if (auto te = rr.as<TestEvent>()) {
                    assert(cnt++ == te->event_number());
                }
            }
            if (r.error()) throw "AJS";
        }
    }
}
