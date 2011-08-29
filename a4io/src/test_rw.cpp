#include <iostream>

#include "a4/writer.h"
#include "a4/reader.h"

#include <boost/foreach.hpp>

using namespace std;

class TestEvent : public StreamableTo<a4::io::TestEvent, TestEvent> {
    public:
        virtual MessagePtr get_message() const { 
            auto * e = new a4::io::TestEvent(); 
            e->set_event_number(eno); 
            return MessagePtr(e);
        }
        int eno;
        static TestEvent * from_message(Message &msg) {
            auto te = new TestEvent();;
            te->eno = dynamic_cast<a4::io::TestEvent&>(msg).event_number();
            return te;
        }
};

class TestMetaData : public MetaData, public StreamableTo<a4::io::TestMetaData, TestMetaData> {
    public:
        virtual MessagePtr get_message() const { 
            auto * e = new a4::io::TestMetaData(); 
            e->set_meta_data(data); 
            return MessagePtr(e);
        }
        static TestMetaData * from_message(Message &msg) {
            auto meta = new TestMetaData();
            meta->data = dynamic_cast<a4::io::TestMetaData&>(msg).meta_data();
            return meta;
        }
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

//    reg_class_id<a4::io::TestEvent, TestEvent>();
    cout << "Known content classes: " << endl;

    //BOOST_FOREACH(const std::map<int, from_msg_func>::value_type& e, all_class_ids) {
    BOOST_FOREACH(auto e, all_class_ids) {
        cout << e.first << " - " << hex << (void*)e.second << dec << endl;
    }

    {
        uint32_t clsid = a4::io::TestEvent::kCLASSIDFieldNumber;
        Writer w("test.a4", "TestEvent", clsid);

        const int N = 1000;
        TestEvent e;
        for(int i = 0; i < N; i++) {
            e.eno = i;
            w.write(e);
        }
        TestMetaData m;
        m.data = N;
        w.metadata(m);
    }
    {
        Reader r("test.a4");
        bool running = true;

        int cnt = 0;
        while (r.is_good()) {
            ReadResult rr = r.read();
            if (rr.class_id == a4::io::TestEvent::kCLASSIDFieldNumber) {
                auto te = boost::dynamic_pointer_cast<TestEvent>(rr.object);
                assert(cnt++ == te->eno);
            } else if (rr.class_id == a4::io::TestMetaData::kCLASSIDFieldNumber) {
                auto meta = boost::dynamic_pointer_cast<TestMetaData>(rr.object);
                assert(cnt == meta->data);
            } else if (rr == READ_ERROR) throw "AJS";
        }
    }
}
