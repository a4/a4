#include "a4/object_store.h"
#include "a4/object_store_impl.h"
#include "a4/storable.h"

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

using namespace std;
using namespace a4::process;

class Results {};

class TestStringThing : public Storable {
    public:
        TestStringThing() : _initialized(false) {}
        virtual shared<const google::protobuf::Message> as_message() { 
            return shared<google::protobuf::Message>(); 
        }
        virtual void construct_from(const google::protobuf::Message&) {}
        virtual void construct_from(shared<google::protobuf::Message>) {}
        Storable&  operator+=(const a4::process::Storable&) { return *this; }
        Storable&  operator*=(const a4::process::Storable&) { return *this; }
        Storable&  operator*=(const double&) { return *this; }
        Storable&& clone_storable() { return std::move(TestStringThing(*this)); }

        TestStringThing& operator()(const int &nr) {
            if (_initialized) return *this;
            _initialized = true;
            n = nr;
            return *this;
        }
        bool _initialized;
        void foo() { n++; }
        string s;
        int n;
};

void process(ObjectStore S, const char* pf) {
    S.T<TestStringThing>(1,pf)(0).foo();
    S.T<TestStringThing>(2,pf)(0).foo();
    S.T<TestStringThing>(3,pf)(0).foo();
    S.T<TestStringThing>(4,pf)(0).foo();
    S.T<TestStringThing>(5,pf)(0).foo();
    S.T<TestStringThing>(6,pf)(0).foo();
    S.T<TestStringThing>(7,pf)(0).foo();
    S.T<TestStringThing>(8,pf)(0).foo();
    S.T<TestStringThing>(9,pf)(0).foo();
    S.T<TestStringThing>(pf,"10")(0).foo();
    S.T<TestStringThing>(pf,"11")(0).foo();
    S.T<TestStringThing>(pf,"12")(0).foo();
    S.T<TestStringThing>(pf,"13")(0).foo();
    S.T<TestStringThing>(pf,"14")(0).foo();
    S.T<TestStringThing>(pf, pf,"15")(0).foo();
    S.T<TestStringThing>(pf,"16")(0).foo();
    S.T<TestStringThing>(pf,"17")(0).foo();
    S.T<TestStringThing>(pf,"18")(0).foo();
    S.T<TestStringThing>(pf,"19")(0).foo();
    S.T<TestStringThing>(pf,"20")(0).foo();
    S.T<TestStringThing>(pf,"21")(0).foo();
    S.T<TestStringThing>(pf,"22")(0).foo();
    S.T<TestStringThing>(pf,"23")(0).foo();
    S.T<TestStringThing>(pf,"24")(0).foo();
    S.T<TestStringThing>(pf,"25")(0).foo();
    S.T<TestStringThing>(pf,"26")(0).foo();
    S.T<TestStringThing>(pf,"27")(0).foo();
    S.T<TestStringThing>(pf,"28")(0).foo();
    S.T<TestStringThing>(pf,"29")(0).foo();
    S.T<TestStringThing>(pf,"30")(0).foo();
    S.T<TestStringThing>(pf,"31")(0).foo();
    S.T<TestStringThing>(pf,"32")(0).foo();
    S.T<TestStringThing>(pf,"33")(0).foo();
    S.T<TestStringThing>(pf,"34")(0).foo();
    S.T<TestStringThing>(pf,"35")(0).foo();
    S.T<TestStringThing>(pf,"36")(0).foo();
    S.T<TestStringThing>(pf,"37")(0).foo();
    S.T<TestStringThing>(pf,"38")(0).foo();
    S.T<TestStringThing>(pf,"39")(0).foo();
    S.T<TestStringThing>(pf,"40")(0).foo();
    S.T<TestStringThing>(pf,"41")(0).foo();
    S.T<TestStringThing>(pf,"42")(0).foo();
    S.T<TestStringThing>(pf,"43")(0).foo();
    S.T<TestStringThing>(pf,"44")(0).foo();
    S.T<TestStringThing>(pf,"45")(0).foo();
    S.T<TestStringThing>(pf,"46")(0).foo();
    S.T<TestStringThing>(pf,"47")(0).foo();
    S.T<TestStringThing>(pf,"48")(0).foo();
    S.T<TestStringThing>(pf,"49")(0).foo();
    S.T<TestStringThing>(pf,"50")(0).foo();
}


//const int N = 1*1000*1000;
const int N = 100*1000;

int main(int argv, char ** argc) {

    ObjectBackStore backstore;
    ObjectStore S = backstore.store();

    assert(!is_writeable_pointer("test"));
    string a = "50";
    assert(is_writeable_pointer(a.c_str()));
    a[1] = '1';
    assert(is_writeable_pointer(a.c_str()));

    for(int i = 0; i < N; i++) {
        process(S, "p1f_");
        process(S, "p2f_");
        process(S, "p3f_");
        process(S, "p4f_");
        process(S, "p5f_");
        process(S, "p6f_");
        process(S, "p7f_");
        process(S, "p8f_");
        process(S, "p9f_");
        process(S, "p10f_");
        process(S, "p11f_");
    }

    foreach(string name, backstore.list<TestStringThing>()) {
        int npro = S.find_slow<TestStringThing>(name)->n;
        if (npro != N) {
            ERROR("Uh oh: ", name, " has ", npro, " != ", N);
            assert(npro == N);
        }
    }
    std::cout << "Test successful" << std::endl;
    return 0;
}
