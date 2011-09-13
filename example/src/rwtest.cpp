#include <iostream>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output_stream.h"
#include "a4/input_stream.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    const int N = 1000;
    {
        uint32_t clsid = TestEvent::kCLASSIDFieldNumber;
        uint32_t clsid_m = TestMetaData::kCLASSIDFieldNumber;
        A4OutputStream w("test.a4", "TestEvent", clsid, clsid_m);

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
        A4InputStream stream("test.a4");

        int cnt = 0;
        while (A4Message msg = stream.next()) {
            if (auto te = msg.as<TestEvent>()) {
                auto me = stream.current_metadata().as<TestMetaData>();
                assert(cnt++ == te->event_number());
                assert(me->meta_data() == N);
            }
        }
        if (stream.error()) throw "AJS";
    }
}
