#include <iostream>

#include "a4/writer.h"
#include "a4/reader.h"

#include <boost/foreach.hpp>

using namespace std;

class TestEvent : public StreamableTo<a4::io::TestEvent, TestEvent> {
    public:
        virtual void to_pb() const { pb->set_event_number(eno); };
        virtual void from_pb() { eno = pb->event_number(); };
        int eno;
};

class TestMetaData : public MetaData, public StreamableTo<a4::io::TestMetaData, TestMetaData> {
    public:
        virtual void to_pb() const { pb->set_meta_data(data); };
        virtual void from_pb() { data = pb->meta_data(); };
        int data;

        TestMetaData & __add__(const Addable & a) {
            data += dynamic_cast<const TestMetaData&>(a).data;
        }

        virtual MetaDataDifference difference(MetaData& a) {
            return MetaData::IDENTICAL;
        }

        virtual Addable * clone() const {return clone_via_message();};
};

int main(int argc, char ** argv) {
    std::string fn;
    if (argc == 1) {
        fn = "test.a4";
    } else if (argc == 2) {
        fn = argv[1];
    } else assert(argc <= 2);

    uint32_t clsid = a4::io::TestEvent::kCLASSIDFieldNumber;
    uint32_t clsid_m = a4::io::TestMetaData::kCLASSIDFieldNumber;
    Writer w(fn, "TestEvent", clsid, clsid_m);

    const int N = 1000;
    TestEvent e;
    for(int i = 0; i < N; i++) {
        e.eno = i;
        w.write(e);
    }
    TestMetaData m;
    m.data = 1;
    w.metadata(m);
    for(int i = 0; i < N; i++) {
        e.eno = N + i;
        w.write(e);
    }
    m.data = 2;
    w.metadata(m);
}
