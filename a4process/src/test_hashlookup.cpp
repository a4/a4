#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <string.h>
#include <stdexcept>
#include <stdarg.h>
#include <assert.h>
#include <a4/hash_lookup.h>

using namespace std;

std::vector<std::string> items;


template <typename... Args>
bool check_set(hash_lookup * h, std::string s, const Args& ...args) {
    string * & res = (string*&)h->lookup(args...);
    if (res == NULL) {
        res = new string(s);
    } else if (*res!=s) {
        cerr << "Uh-oh: " << res << " != " << s << endl;
        return false;
    }
    return true;
}

int main(int argv, char ** argc) {

    // Test the writeable pointer function
    assert(!is_writeable_pointer("test"));
    string a = "50";
    assert(is_writeable_pointer(a.c_str()));
    a[1] = '1';
    assert(is_writeable_pointer(a.c_str()));

    hash_lookup * hl = new hash_lookup();

    check_set(hl, "test", "test");
    return 0;
}
