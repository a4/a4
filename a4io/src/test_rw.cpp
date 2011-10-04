#include <iostream>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output_stream.h"
#include "a4/input_stream.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    {
        A4OutputStream w("test_rw.a4", "TestEvent");
        w.content_cls<TestEvent>().metadata_cls<TestMetaData>();

        const int N = 1000;
        TestEvent e;
        for(int i = 0; i < N; i++) {
            e.set_event_number(i);
            w.write(e);
        }
        TestMetaData m;
        m.set_meta_data(N);
        w.write(m);

    }
    {
        A4InputStream r("test_rw.a4");
        int cnt = 0;
        while (A4Message msg = r.next()) {
            if (shared<TestEvent> te = msg.as<TestEvent>()) {
                assert(cnt++ == te->event_number());
            }
        }
        if (r.error()) throw "AJS";
    }
    return 0;
}
