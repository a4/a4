#include <iostream>

#include "a4/io/A4Stream.pb.h"
#include "a4/output_stream.h"
#include "a4/input_stream.h"

#include <gtest/gtest.h>

using namespace std;
using namespace a4::io;

TEST(a4io, read_write) {
    {
        OutputStream w("test_rw.a4", "TestEvent");

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
        InputStream r("test_rw.a4");
        int cnt = 0;
        while (A4Message msg = r.next()) {
            if (const TestEvent* te = msg.as<TestEvent>()) {
                assert(cnt++ == te->event_number());
            }
        }
        ASSERT_FALSE(r.error());
    }
}
