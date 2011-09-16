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

using namespace std;

std::vector<std::string> items;

template <typename... Args>
void lookup1000(hash_lookup * hl) {
    const char * dirs[] = {"alpha/", "beta/", "gamma/", "delta/", "epsilon/", "phi/", "chi/", "xi/", "tau/", "omikron/"};
    for (int i = 0; i < 10; i++) {
        hl->lookup(dirs[i], "A00");
        hl->lookup(dirs[i], "A01");
        hl->lookup(dirs[i], "A02");
        hl->lookup(dirs[i], "A03");
        hl->lookup(dirs[i], "A04");
        hl->lookup(dirs[i], "A05");
        hl->lookup(dirs[i], "A06");
        hl->lookup(dirs[i], "A07");
        hl->lookup(dirs[i], "A08");
        hl->lookup(dirs[i], "A09");
        hl->lookup(dirs[i], "A10");
        hl->lookup(dirs[i], "A11");
        hl->lookup(dirs[i], "A12");
        hl->lookup(dirs[i], "A13");
        hl->lookup(dirs[i], "A14");
        hl->lookup(dirs[i], "A15");
        hl->lookup(dirs[i], "A16");
        hl->lookup(dirs[i], "A17");
        hl->lookup(dirs[i], "A18");
        hl->lookup(dirs[i], "A19");
        hl->lookup(dirs[i], "A20");
        hl->lookup(dirs[i], "A21");
        hl->lookup(dirs[i], "A22");
        hl->lookup(dirs[i], "A23");
        hl->lookup(dirs[i], "A24");
        hl->lookup(dirs[i], "A25");
        hl->lookup(dirs[i], "A26");
        hl->lookup(dirs[i], "A27");
        hl->lookup(dirs[i], "A28");
        hl->lookup(dirs[i], "A29");
        hl->lookup(dirs[i], "A30");
        hl->lookup(dirs[i], "A31");
        hl->lookup(dirs[i], "A32");
        hl->lookup(dirs[i], "A33");
        hl->lookup(dirs[i], "A34");
        hl->lookup(dirs[i], "A35");
        hl->lookup(dirs[i], "A36");
        hl->lookup(dirs[i], "A37");
        hl->lookup(dirs[i], "A38");
        hl->lookup(dirs[i], "A39");
        hl->lookup(dirs[i], "A40");
        hl->lookup(dirs[i], "A41");
        hl->lookup(dirs[i], "A42");
        hl->lookup(dirs[i], "A43");
        hl->lookup(dirs[i], "A44");
        hl->lookup(dirs[i], "A45");
        hl->lookup(dirs[i], "A46");
        hl->lookup(dirs[i], "A47");
        hl->lookup(dirs[i], "A48");
        hl->lookup(dirs[i], "A49");
        hl->lookup(dirs[i], "A50");
        hl->lookup(dirs[i], "A51");
        hl->lookup(dirs[i], "A52");
        hl->lookup(dirs[i], "A53");
        hl->lookup(dirs[i], "A54");
        hl->lookup(dirs[i], "A55");
        hl->lookup(dirs[i], "A56");
        hl->lookup(dirs[i], "A57");
        hl->lookup(dirs[i], "A58");
        hl->lookup(dirs[i], "A59");
        hl->lookup(dirs[i], "A60");
        hl->lookup(dirs[i], "A61");
        hl->lookup(dirs[i], "A62");
        hl->lookup(dirs[i], "A63");
        hl->lookup(dirs[i], "A64");
        hl->lookup(dirs[i], "A65");
        hl->lookup(dirs[i], "A66");
        hl->lookup(dirs[i], "A67");
        hl->lookup(dirs[i], "A68");
        hl->lookup(dirs[i], "A69");
        hl->lookup(dirs[i], "A70");
        hl->lookup(dirs[i], "A71");
        hl->lookup(dirs[i], "A72");
        hl->lookup(dirs[i], "A73");
        hl->lookup(dirs[i], "A74");
        hl->lookup(dirs[i], "A75");
        hl->lookup(dirs[i], "A76");
        hl->lookup(dirs[i], "A77");
        hl->lookup(dirs[i], "A78");
        hl->lookup(dirs[i], "A79");
        hl->lookup(dirs[i], "A80");
        hl->lookup(dirs[i], "A81");
        hl->lookup(dirs[i], "A82");
        hl->lookup(dirs[i], "A83");
        hl->lookup(dirs[i], "A84");
        hl->lookup(dirs[i], "A85");
        hl->lookup(dirs[i], "A86");
        hl->lookup(dirs[i], "A87");
        hl->lookup(dirs[i], "A88");
        hl->lookup(dirs[i], "A89");
        hl->lookup(dirs[i], "A90");
        hl->lookup(dirs[i], "A91");
        hl->lookup(dirs[i], "A92");
        hl->lookup(dirs[i], "A93");
        hl->lookup(dirs[i], "A94");
        hl->lookup(dirs[i], "A95");
        hl->lookup(dirs[i], "A96");
        hl->lookup(dirs[i], "A97");
        hl->lookup(dirs[i], "A98");
        hl->lookup(dirs[i], "A99");
    }
}

template <typename... Args>
void check_set(hash_lookup * h, const Args& ...args) {
    string * & res = (string*&)h->lookup(args...);
    assert(res != NULL);
    assert(*res == str_cat(args...));
}

template <typename... Args>
void test_check_set(hash_lookup * h, const Args& ...args) {
    test_set(h, args...);
    check_set(h, args...);
}

int main(int argv, char ** argc) {
    const int N = 1000;
    const int M = 1000;
    hash_lookup * h = new hash_lookup();
    for (int i = 0; i < N; i++) for(int j = 0; j < M; j++) lookup1000(h->subhash("test/", i%2, "/", j%5, "/"));
    std::cout << 1000*N*M << std::endl;
    return 0;
}
