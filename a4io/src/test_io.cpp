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

        shared<A4OutputStream> stream = a4o.get_stream();
        stream->content_cls<TestEvent>();
        stream->metadata_cls<TestMetaData>();

        TestEvent e;
        for(int i = 0; i < N; i++) {
            e.set_event_number(i);
            stream->write(e);
        }
        TestMetaData m;
        m.set_meta_data(N);
        stream->metadata(m);
    }
    {
        A4Input in;
        in.add_file("test_io.a4");
        in.add_file("test_io.a4");
        int cnt = 0;
        while (shared<A4InputStream> stream = in.get_stream()) {
            while (A4Message msg = stream->next()) {
                if (shared<TestEvent> te = msg.as<TestEvent>()) {
                    assert((cnt++%N) == te->event_number());
                }
            }
            if (stream->error()) throw "AJS";
        }
        assert(cnt == 2*N);
    }
}
