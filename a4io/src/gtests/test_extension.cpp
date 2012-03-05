#include <iostream>

#include "a4/io/A4Stream.pb.h"
#include "a4/io/A4Extension.pb.h"

#include "a4/output_stream.h"
#include "a4/input_stream.h"

#include <gtest/gtest.h>

using namespace std;
using namespace a4::io;

const int N = 1000;

TEST(a4io, extension) {
    {
        OutputStream w("test_ext.a4", "TestEvent");

        TestEvent e;
        for(int i = 0; i < N; i++) {
            e.set_event_number(i);
            e.SetExtension(TestEventExt::my_extension_value, i);
            w.write(e);
        }

    }
    {
        InputStream r("test_ext.a4");
        int cnt = 0;
        while (shared<A4Message> msg = r.next()) {
            if (const TestEvent* te = msg->as<TestEvent>()) {
                ASSERT_EQ(cnt++, te->GetExtension(TestEventExt::my_extension_value));
            }
        }
    }
}

