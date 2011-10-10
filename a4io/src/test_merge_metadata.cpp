#include <iostream>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output_stream.h"
#include "a4/input_stream.h"

#include <gtest/gtest.h>

using namespace std;
using namespace a4::io;

TestMetaData meta(int d, int run, int lb, int period) {
    TestMetaData m;
    m.set_meta_data(d);
    TestRunLB * rlb = m.add_run_data();
    rlb->set_run(run); rlb->set_lumiblock(lb);
    m.add_period(period);
    return m;
}

TEST(a4io, metadata_merge) {
    {
        A4OutputStream w("test_rw.a4", "TestEvent");
        w.content_cls<TestEvent>().metadata_cls<TestMetaData>();

        const int N = 1000;
        int cnt = 0;
        TestEvent e;
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.write(meta(1,1,1,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.write(meta(2,1,2,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.write(meta(3,2,1,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.write(meta(4,2,2,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.write(meta(5,3,1,2));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.write(meta(6,3,2,2));
    }
    {
        A4InputStream r("test_rw.a4");
        int cnt = 0;
        A4Message current_md;
        while (A4Message msg = r.next()) {

            if (shared<TestEvent> te = msg.as<TestEvent>()) {
                assert(cnt++ == te->event_number());
            }
            if (r.new_metadata()) {
                A4Message new_md = r.current_metadata();
                if (current_md) {
                    std::cout << "CURRENT:\n" << current_md.message->ShortDebugString() << std::endl;
                    std::cout << "NEXT   :\n" << new_md.message->ShortDebugString() << std::endl;
                    unique<TestMetaData> merged_md = r.merge_metadata(current_md, new_md);
                    std::cout << "MERGED :\n" << merged_md->ShortDebugString() << std::endl;
                }
                current_md = new_md;
            }
        }
        ASSERT_FALSE(r.error());
    }
}
