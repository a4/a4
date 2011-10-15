#include <iostream>

#include "a4/output_stream.h"
#include "a4/input_stream.h"
#include "A4Key.pb.h"

using namespace std;
using namespace a4::io;
using namespace a4::process;

int main(int argc, char ** argv) {
    {
        OutputStream w("test_histos.a4", "TestHisto");
        w.content_cls<TestHisto>().metadata_cls<TestHistoMetaData>();

        const int N = 1000;
        TestHisto e;
        for(int i = 0; i < N; i++) {
            e.set_bin_number(i);
            w.write(e);
        }
        TestHistoMetaData m;
        m.set_meta_data(N);
        w.write(m);

    }
    {
        InputStream r("test_histos.a4");
        int cnt = 0;
        while (A4Message msg = r.next()) {
            if (shared<TestHisto> te = msg.as<TestHisto>()) {
                assert(cnt++ == te->bin_number());
            }
        }
        if (r.error()) throw "AJS";
    }
    return 0;
}
