#include <iostream>

#include <iostream>

#include <a4/application.h>
#include <a4/process/A4Key.pb.h>
#include <a4/io/A4Stream.pb.h>

#include "a4/output_stream.h"

#include <gtest/gtest.h>

using namespace std;
using namespace a4::io;
using namespace a4::process;

TestMergeMetaData meta(int d, int run, int lb, int period, bool simulation=false, bool error=false) {
    TestMergeMetaData m;
    m.set_meta_data(d);
    //TestRunLB* rlb = m.add_lumiblock();
    //rlb->set_run(run); rlb->set_lumiblock(lb);
    m.add_run(run);
    m.add_period(period);
    m.set_simulation(simulation);
    m.add_comments("Hugo!");
    m.set_tags("<tag>");
    m.set_efficiency(0.5);
    m.set_error(error);
    return m;
}


class IdentityProcessor : public ProcessorOf<TestEvent, TestMergeMetaData> {
public:
    void process(const TestEvent& e) {
        //DEBUG("Processing event.");
        write(e);
    }
};

class CheckMetadataProcessor : public ProcessorOf<TestEvent, TestMergeMetaData> {
public:
    void process_new_metadata() {
        auto& m = metadata();
        DEBUG("Can haz metadata? ", m.run_size(), " - ", m.lumiblock_size());
    }
    void process(const TestEvent& e) {
    }
};



TEST(a4process, metadata_merge_union) {
    
    const size_t nmeta = 100;
    
    {
        OutputStream w("test_metadata_input.a4", "TestEvent");
        
        TestEvent e;
        
        // Fill many different periods (which are merge_union)
        for (size_t i = 0; i < nmeta; i++) {
            w.metadata(meta(1, i % 5, 1, i));
            w.write(e);
        }
    }
    
    {
        const char* args[] = {
            "gtests", "test_metadata_input.a4", 
            "--per", "simulation", 
            "-o", "test_metadata_output.a4"
        };
        a4_main_process<IdentityProcessor>(sizeof(args)/sizeof(char*), args);
    }
    
    {
        const char* args[] = {
            "gtests", "test_metadata_output.a4"
        };
        a4_main_process<CheckMetadataProcessor>(sizeof(args)/sizeof(char*), args);
    }
}
