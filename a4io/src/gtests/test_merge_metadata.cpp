#include <iostream>

#include <a4/io/A4Stream.pb.h>
#include <a4/message.h>
#include <a4/output_stream.h>
#include <a4/input_stream.h>

#include <gtest/gtest.h>

using namespace std;
using namespace a4::io;

TestMergeMetaData meta(int d, int run, int lb, int period, bool simulation=false, bool error=false) {
    TestMergeMetaData m;
    m.set_meta_data(d);
    TestRunLB* rlb = m.add_lumiblock();
    rlb->set_run(run); rlb->set_lumiblock(lb);
    m.add_run(run);
    m.add_period(period);
    m.set_simulation(simulation);
    m.add_comments("Hugo!");
    m.set_tags("<tag>");
    m.set_efficiency(0.5);
    m.set_error(error);
    return m;
}

TEST(a4io, check_mergable) {
    auto data_m1(meta(0, 123456, 100, 0, false)),
         data_m2(meta(0, 123456, 100, 0, false)),
         mc_m1(meta(0, 123456, 100, 0, true)),
         mc_m2(meta(0, 123457, 100, 0, true));

    ASSERT_EQ(data_m1.run(0), 123456);

    #define AM A4Message

    ASSERT_TRUE(AM(data_m1).check_key_mergable(AM(data_m2), "run"));
    ASSERT_TRUE(AM(data_m1).check_key_mergable(AM(data_m2), "simulation"));
    
    ASSERT_FALSE(AM(data_m1).check_key_mergable(AM(mc_m1), "simulation"));
    
    ASSERT_FALSE(AM(mc_m1).check_key_mergable(AM(mc_m2), "run"));
    
    A4Message merged(data_m1);
    
    merged += AM(data_m2);
    
    EXPECT_THROW(merged += AM(mc_m1), a4::Fatal);
    
    #undef AM
}

TEST(a4io, metadata_merge) {
    {
        OutputStream w("test_mm.a4", "TestEvent");

        const int N = 1000;
        int cnt = 0;
        TestEvent e;
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.metadata(meta(1,1,1,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.metadata(meta(1,10,1,1, true)); // will cause an exception
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.metadata(meta(2,1,2,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.metadata(meta(3,2,1,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.metadata(meta(4,2,2,1));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.metadata(meta(5,3,1,2));
        for(int i = 0; i < N; i++) {e.set_event_number(cnt++); w.write(e);};
        w.metadata(meta(6,3,2,2));
    }
    {
        InputStream r("test_mm.a4");
        int cnt = 0;
        shared<A4Message> current_md;
        int mcnt = 0;
        while (shared<A4Message> msg = r.next()) {

            if (const TestEvent* te = msg->as<TestEvent>()) {
                assert(cnt++ == te->event_number());
            }
            if (r.new_metadata()) {
                shared<A4Message> new_md(new A4Message(*r.current_metadata()));
                if (current_md) {
                    //std::cout << "CURRENT:\n" << current_md->message()->ShortDebugString() << std::endl;
                    //std::cout << "NEXT   :\n" << new_md->message()->ShortDebugString() << std::endl;
                    if (mcnt == 1) {
                        A4Message test_sum(*current_md);
                        EXPECT_THROW(std::cout << "ERRONEOUS: " << (test_sum += *new_md).message()->ShortDebugString() << std::endl;, a4::Fatal);
                    } else {
                        *current_md += *new_md;
                        //std::cout << "MERGED :\n" << current_md->message()->ShortDebugString() << std::endl;
                    }
                } else current_md = new_md;
                mcnt++;
            }
        }
        ASSERT_FALSE(r.error());
    }
}
