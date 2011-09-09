#include <iostream>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output_stream.h"
#include "a4/input_stream.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    const int mod = 1024*1024;
    {
        uint32_t clsid = TestEvent::kCLASSIDFieldNumber;
        uint32_t clsid_m = TestMetaData::kCLASSIDFieldNumber;
        A4OutputStream w("test.a4", "TestEvent", clsid, clsid_m, false, false);

        const uint64_t N = 800*1000*1000L;
        //const uint64_t N = 5*1024;
        cout << "Writing " << N << " events..." << endl;

        TestEvent e;
        for(uint64_t i = 0; i < N; i++) {
            if (i % mod == 0) cout << "..."<< i << endl;
            e.set_event_number(i%mod);
            w.write(e);
        }
        TestMetaData m;
        m.set_meta_data(mod);
        w.metadata(m);
        cout << "...done." << endl;
    }
}
