#include <iostream>

#include "a4/writer.h"
#include "a4/reader.h"

#include "a4/proto/io/A4Stream.pb.h"

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

    Reader r(fn);

    while (r.is_good()) {
        ReadResult rr = r.read();
        if (rr.class_id == TestEvent::class_id) {
            auto te = boost::dynamic_pointer_cast<TestEvent>(rr.object);
            std::cout << "TestEvent: " << te->eno << std::endl;
        } else if (rr.class_id == TestMetaData::class_id) {
            auto meta = boost::dynamic_pointer_cast<TestMetaData>(rr.object);
            std::cout << "TestMetaData: " << meta->data << std::endl;
        } else if (rr == READ_ERROR) throw "up";
    }
}
