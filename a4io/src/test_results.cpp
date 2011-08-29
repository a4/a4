#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <string.h>
#include <stdexcept>
#include <stdarg.h>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include "a4/proto/io/A4Stream.pb.h"

#include "a4/objectstore.h"
#include "a4/streamable.h"
#include "a4/results.h"
#include "a4/reader.h"
#include "a4/writer.h"

using namespace std;

class TestEvent : public Named, public CallConstructible, public StreamableTo<a4::io::TestEvent, TestEvent> {
    public:
        static TestEvent * from_message(Message & msg) {
            a4::io::TestEvent& evt = dynamic_cast<a4::io::TestEvent&>(msg);
            return new TestEvent();
        };
        virtual MessagePtr get_message() const {
            return MessagePtr(new a4::io::TestEvent());
        };

        virtual TestEvent & operator()(int _a, int _b, int _c) {
            a = _a;
            b = _b;
            c = _c;
            return *this;
        }
        void fill(double f) { val += f; };

        int a, b, c;
        float val;
};


void process(Results & r, const char * pf) {
    r.get_cat<TestEvent>(1,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(2,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(3,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(4,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(5,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(6,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(7,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(8,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(9,pf)(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"10")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"11")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"12")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"13")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"14")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf, pf,"15")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"16")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"17")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"18")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"19")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"20")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"21")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"22")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"23")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"24")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"25")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"26")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"27")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"28")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"29")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"30")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"31")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"32")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"33")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"34")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"35")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"36")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"37")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"38")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"39")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"40")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"41")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"42")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"43")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"44")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"45")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"46")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"47")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"48")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"49")(100,0,1).fill(0.42);
    r.get_cat<TestEvent>(pf,"50")(100,0,1).fill(0.42);
}


const int N = 1*1000*1000;

int main(int argv, char ** argc) {

    Results * r = new Results();

    assert(!is_writeable_pointer("test"));
    string a = "50";
    assert(is_writeable_pointer(a.c_str()));
    a[1] = '1';
    assert(is_writeable_pointer(a.c_str()));

    for(int i = 0; i < N; i++) {
        process(*r, "p1f_");
        process(*r, "p2f_");
        process(*r, "p3f_");
        process(*r, "p4f_");
        process(*r, "p5f_");
        process(*r, "p6f_");
        process(*r, "p7f_");
        process(*r, "p8f_");
        process(*r, "p9f_");
        process(*r, "p10f_");
        process(*r, "p11f_");
    }

    auto w = new Writer("test.results");
    w->write(*r->get_checked<TestEvent>("1p1f_"));
    delete w;
    
    std::cout << "Test successful" << std::endl;
    return 0;
}
