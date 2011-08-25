
#include "a4/objectstore.h"
#include "a4/results.h"
#include "a4/h1.h"

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

void process(Results & r, const char * pf) {
    r.get_cat<H1>(1,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(2,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(3,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(4,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(5,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(6,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(7,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(8,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(9,pf)(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"10")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"11")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"12")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"13")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"14")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf, pf,"15")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"16")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"17")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"18")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"19")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"20")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"21")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"22")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"23")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"24")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"25")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"26")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"27")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"28")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"29")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"30")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"31")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"32")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"33")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"34")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"35")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"36")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"37")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"38")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"39")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"40")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"41")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"42")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"43")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"44")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"45")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"46")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"47")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"48")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"49")(100,0,1).fill(0.42);
    r.get_cat<H1>(pf,"50")(100,0,1).fill(0.42);
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
    std::cout << "Test successful" << std::endl;
    return 0;
}
