
#include "a4/objectstore.h"

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

using namespace std;

class Storable {
    public:
        Storable() : _initialized(false) {};
        virtual bool is_initialized() { return _initialized; }

    protected:
        bool _initialized;
};

class Results : public ObjectStore<Storable> {};

class TestStringThing : public Storable {
    public:
        //TestStringThing(const string s, const int n) : s(s), n(n) {}
        TestStringThing & operator()(const int &nr) {
            if (_initialized) return *this;
            n = nr;
            _initialized = true;
            return *this;
        };
        void foo() { n++; };
        string s;
        int n;
    private:
};

void process(Results & r, const char * pf) {
    r.get_cat<TestStringThing>(1,pf)(0).foo();
    r.get_cat<TestStringThing>(2,pf)(0).foo();
    r.get_cat<TestStringThing>(3,pf)(0).foo();
    r.get_cat<TestStringThing>(4,pf)(0).foo();
    r.get_cat<TestStringThing>(5,pf)(0).foo();
    r.get_cat<TestStringThing>(6,pf)(0).foo();
    r.get_cat<TestStringThing>(7,pf)(0).foo();
    r.get_cat<TestStringThing>(8,pf)(0).foo();
    r.get_cat<TestStringThing>(9,pf)(0).foo();
    r.get_cat<TestStringThing>(pf,"10")(0).foo();
    r.get_cat<TestStringThing>(pf,"11")(0).foo();
    r.get_cat<TestStringThing>(pf,"12")(0).foo();
    r.get_cat<TestStringThing>(pf,"13")(0).foo();
    r.get_cat<TestStringThing>(pf,"14")(0).foo();
    r.get_cat<TestStringThing>(pf, pf,"15")(0).foo();
    r.get_cat<TestStringThing>(pf,"16")(0).foo();
    r.get_cat<TestStringThing>(pf,"17")(0).foo();
    r.get_cat<TestStringThing>(pf,"18")(0).foo();
    r.get_cat<TestStringThing>(pf,"19")(0).foo();
    r.get_cat<TestStringThing>(pf,"20")(0).foo();
    r.get_cat<TestStringThing>(pf,"21")(0).foo();
    r.get_cat<TestStringThing>(pf,"22")(0).foo();
    r.get_cat<TestStringThing>(pf,"23")(0).foo();
    r.get_cat<TestStringThing>(pf,"24")(0).foo();
    r.get_cat<TestStringThing>(pf,"25")(0).foo();
    r.get_cat<TestStringThing>(pf,"26")(0).foo();
    r.get_cat<TestStringThing>(pf,"27")(0).foo();
    r.get_cat<TestStringThing>(pf,"28")(0).foo();
    r.get_cat<TestStringThing>(pf,"29")(0).foo();
    r.get_cat<TestStringThing>(pf,"30")(0).foo();
    r.get_cat<TestStringThing>(pf,"31")(0).foo();
    r.get_cat<TestStringThing>(pf,"32")(0).foo();
    r.get_cat<TestStringThing>(pf,"33")(0).foo();
    r.get_cat<TestStringThing>(pf,"34")(0).foo();
    r.get_cat<TestStringThing>(pf,"35")(0).foo();
    r.get_cat<TestStringThing>(pf,"36")(0).foo();
    r.get_cat<TestStringThing>(pf,"37")(0).foo();
    r.get_cat<TestStringThing>(pf,"38")(0).foo();
    r.get_cat<TestStringThing>(pf,"39")(0).foo();
    r.get_cat<TestStringThing>(pf,"40")(0).foo();
    r.get_cat<TestStringThing>(pf,"41")(0).foo();
    r.get_cat<TestStringThing>(pf,"42")(0).foo();
    r.get_cat<TestStringThing>(pf,"43")(0).foo();
    r.get_cat<TestStringThing>(pf,"44")(0).foo();
    r.get_cat<TestStringThing>(pf,"45")(0).foo();
    r.get_cat<TestStringThing>(pf,"46")(0).foo();
    r.get_cat<TestStringThing>(pf,"47")(0).foo();
    r.get_cat<TestStringThing>(pf,"48")(0).foo();
    r.get_cat<TestStringThing>(pf,"49")(0).foo();
    r.get_cat<TestStringThing>(pf,"50")(0).foo();
}


//const int N = 1*1000*1000;
const int N = 100*1000;

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
    BOOST_FOREACH(string name, r->list<TestStringThing>()) {
        int npro = r->get_checked<TestStringThing>(name)->n;
        if (npro != N) {
            std::cerr << "Uh oh: " << name << " has " << npro << " != " << N << std::endl;
            assert(npro == N);
        }
    }
    std::cout << "Test successful" << std::endl;
    return 0;
}
