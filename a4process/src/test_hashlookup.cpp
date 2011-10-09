#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <string.h>
#include <stdexcept>
#include <stdarg.h>
#include <cassert>

#include <a4/hash_lookup.h>
#include <a4/types.h>

using namespace std;

std::vector<std::string> items;

template <typename... Args>
void test_set(hash_lookup * h, const Args& ...args) {
    string * & res = (string*&)h->lookup(args...);
    assert(res == NULL);
    res = new string(str_cat(args...));
}

template <typename... Args>
void check_set(hash_lookup * h, const Args& ...args) {
    string * & res = (string*&)h->lookup(args...);
    assert(res != NULL);
    assert(*res == str_cat(args...));
    delete res;
    res = 0;
}

template <typename... Args>
void test_check_set(hash_lookup * h, const Args& ...args) {
    test_set(h, args...);
    check_set(h, args...);
}

int main(int argv, char ** argc) {

    // Test the writeable pointer function
    assert(!is_writeable_pointer("test"));
    string a = "50";
    assert(is_writeable_pointer(a.c_str()));
    a[1] = '1';
    assert(is_writeable_pointer(a.c_str()));

    unique<hash_lookup> hl(new hash_lookup());

    // Test the hash lookup for consistency
    test_check_set(hl.get(), "test");
    test_check_set(hl.get(), "test", 1);
    test_check_set(hl.get(), "test", 1, "A");
    for (int i = 0; i < (1<<16); i++) test_check_set(hl.get(), "test", 1, "A", i);
    for (int i = 0; i < (1<<16); i++) test_check_set(hl.get(), "test", 2, "A", i);
    return 0;
}
