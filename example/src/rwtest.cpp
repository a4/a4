#include <iostream>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output.h"
#include "a4/input.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    const int N = 1000;
    {
        A4Output a4o("test_io.a4", "TestEvent");

        auto stream = a4o.get_stream();
        stream->content_cls<TestEvent>();
        stream->metadata_cls<TestMetaData>();

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
        while (shared<A4InputStream> stream = in.get_stream()) {
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
}

