#include <iostream>

#include "a4/writer.h"
#include "a4/writer_impl.h"

#include "a4/reader.h"
#include "a4/reader_impl.h"

#include "pb/A4Stream.pb.h"

using namespace std;

int main(int argc, char ** argv) {
    {
        Writer w("test.a4", "TestEvent", TestEvent::kCLASSIDFieldNumber);
        TestEvent e;
        e.set_event_number(1);
        w.write(e);
        e.set_event_number(2);
        w.write(e);
        e.set_event_number(3);
        w.write(e);
        e.set_event_number(4);
        w.write(e);
        e.set_event_number(5);
        w.write(e);
        TestMetaData m;
        m.set_meta_data(5);
        w.write_metadata(m);
    }
    {
        Reader<TestEvent, TestMetaData> r("test.a4");
        TestEvent e;
        bool running = true;
        while (running) {
            switch (r.read(e)) {
                case READ_ITEM: cout << e.event_number() << endl; continue;
                case NEW_METADATA: cout << "META: " << r.last_meta_data()->meta_data() << endl; continue;
                case STREAM_END: running=false; break;
                case FAIL:
                    throw "AJS";
            }
            
        }
    }
}
