#include <iostream>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output_stream.h"
#include "a4/input_stream.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    const int mod = 1024*1024;
    {
        cout << "Reading events..." << endl;
        A4InputStream r("test.a4");
        bool running = true;

        int cnt = 0;
        uint64_t i = 0;
        while (r.is_good()) {
            if (i++ % mod == 0) cout << "..."<< i << endl;
            ReadResult rr = r.next();
            if (rr.class_id == TestEvent::kCLASSIDFieldNumber) {
                auto te = dynamic_shared_cast<TestEvent>(rr.object);
                if (te->event_number() == mod-1) cnt=0;
                else assert(cnt++ == te->event_number());
            } else if (rr.class_id == TestMetaData::kCLASSIDFieldNumber) {
                auto meta = dynamic_shared_cast<TestMetaData>(rr.object);
                assert(mod == meta->meta_data());
            } else if (rr == READ_ERROR) throw "AJS";
        }
        cout << "...done." << endl;
    }
}
