#include "a4/application.h"
#include "a4/grl.h"

#include <iostream>
#include <map>
#include <set>
#include <utility>

using namespace std;

int main(int argc, char ** argv) {
    if (argc != 2) {
        cout << "Usage: test_grl <test.grl file>" << endl;
    }
    GRL test(argv[1]);
    cout << test.pass(8888880, 699) << endl;
    cout << test.pass(180710, 699) << endl;
    cout << test.pass(180710, 700) << endl;
    cout << test.pass(180710, 701) << endl;
    cout << test.pass(180710, 709) << endl;
    cout << test.pass(180710, 710) << endl;
    cout << test.pass(180710, 711) << endl;
    return 0;
};
