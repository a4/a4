#include <iostream>
#include <stdlib.h>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output_stream.h"
#include "a4/input_stream.h"

using namespace std;
using namespace a4::io;

int main(int argc, char ** argv) {
    const int mod = 1024*1024;
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " <forward meta> <compression> <filename>" << endl;
        cout << " where <forward meta> and <compression> are 1 or 0 for on and off" << endl;
        return -1;
    }

    const bool forward = atoi(argv[1]) == 1;
    const bool compression = atoi(argv[2]) == 1;
    {
        A4OutputStream w(argv[3], "TestEvent");
        w.content_cls<TestEvent>().metadata_cls<TestMetaData>();
        w.set_compression(compression);
        if (forward) w.set_forward_metadata();

        const uint64_t N = 800*1000*1000L;
        //const uint64_t N = 5*1024;
        cout << "Writing " << N << " events..." << endl;

        TestEvent e;
        TestMetaData m;
        m.set_meta_data(0);
        if (forward) w.write(m);
        for(uint64_t i = 0; i < N; i++) {
            int32_t en = i%mod;
            if (i % 1000000 == 0) cout << "..." << i << endl;
            if (en/1000 != m.meta_data()) {
                if (!forward) w.write(m);
                m.set_meta_data(en/1000);
                if (forward) w.write(m);
            }
            e.set_event_number(en);
            w.write(e);
        }
        if (!forward) w.write(m);
        cout << "...done." << endl;
    }
}
