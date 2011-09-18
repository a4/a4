#include <iostream>
#include <string>
#include <cassert>

#include <a4/object_store.h>
#include <a4/object_store_impl.h>

using namespace std;

std::vector<std::string> items;

template <typename... Args>
void lookup1000(ObjectStore S) {
    const char * dirs[] = {"alpha/", "beta/", "gamma/", "delta/", "epsilon/", "phi/", "chi/", "xi/", "tau/", "omikron/"};
    for (int i = 0; i < 10; i++) {
        S.T<int>(dirs[i], "A00");
        S.T<int>(dirs[i], "A01");
        S.T<int>(dirs[i], "A02");
        S.T<int>(dirs[i], "A03");
        S.T<int>(dirs[i], "A04");
        S.T<int>(dirs[i], "A05");
        S.T<int>(dirs[i], "A06");
        S.T<int>(dirs[i], "A07");
        S.T<int>(dirs[i], "A08");
        S.T<int>(dirs[i], "A09");
        S.T<int>(dirs[i], "A10");
        S.T<int>(dirs[i], "A11");
        S.T<int>(dirs[i], "A12");
        S.T<int>(dirs[i], "A13");
        S.T<int>(dirs[i], "A14");
        S.T<int>(dirs[i], "A15");
        S.T<int>(dirs[i], "A16");
        S.T<int>(dirs[i], "A17");
        S.T<int>(dirs[i], "A18");
        S.T<int>(dirs[i], "A19");
        S.T<int>(dirs[i], "A20");
        S.T<int>(dirs[i], "A21");
        S.T<int>(dirs[i], "A22");
        S.T<int>(dirs[i], "A23");
        S.T<int>(dirs[i], "A24");
        S.T<int>(dirs[i], "A25");
        S.T<int>(dirs[i], "A26");
        S.T<int>(dirs[i], "A27");
        S.T<int>(dirs[i], "A28");
        S.T<int>(dirs[i], "A29");
        S.T<int>(dirs[i], "A30");
        S.T<int>(dirs[i], "A31");
        S.T<int>(dirs[i], "A32");
        S.T<int>(dirs[i], "A33");
        S.T<int>(dirs[i], "A34");
        S.T<int>(dirs[i], "A35");
        S.T<int>(dirs[i], "A36");
        S.T<int>(dirs[i], "A37");
        S.T<int>(dirs[i], "A38");
        S.T<int>(dirs[i], "A39");
        S.T<int>(dirs[i], "A40");
        S.T<int>(dirs[i], "A41");
        S.T<int>(dirs[i], "A42");
        S.T<int>(dirs[i], "A43");
        S.T<int>(dirs[i], "A44");
        S.T<int>(dirs[i], "A45");
        S.T<int>(dirs[i], "A46");
        S.T<int>(dirs[i], "A47");
        S.T<int>(dirs[i], "A48");
        S.T<int>(dirs[i], "A49");
        S.T<int>(dirs[i], "A50");
        S.T<int>(dirs[i], "A51");
        S.T<int>(dirs[i], "A52");
        S.T<int>(dirs[i], "A53");
        S.T<int>(dirs[i], "A54");
        S.T<int>(dirs[i], "A55");
        S.T<int>(dirs[i], "A56");
        S.T<int>(dirs[i], "A57");
        S.T<int>(dirs[i], "A58");
        S.T<int>(dirs[i], "A59");
        S.T<int>(dirs[i], "A60");
        S.T<int>(dirs[i], "A61");
        S.T<int>(dirs[i], "A62");
        S.T<int>(dirs[i], "A63");
        S.T<int>(dirs[i], "A64");
        S.T<int>(dirs[i], "A65");
        S.T<int>(dirs[i], "A66");
        S.T<int>(dirs[i], "A67");
        S.T<int>(dirs[i], "A68");
        S.T<int>(dirs[i], "A69");
        S.T<int>(dirs[i], "A70");
        S.T<int>(dirs[i], "A71");
        S.T<int>(dirs[i], "A72");
        S.T<int>(dirs[i], "A73");
        S.T<int>(dirs[i], "A74");
        S.T<int>(dirs[i], "A75");
        S.T<int>(dirs[i], "A76");
        S.T<int>(dirs[i], "A77");
        S.T<int>(dirs[i], "A78");
        S.T<int>(dirs[i], "A79");
        S.T<int>(dirs[i], "A80");
        S.T<int>(dirs[i], "A81");
        S.T<int>(dirs[i], "A82");
        S.T<int>(dirs[i], "A83");
        S.T<int>(dirs[i], "A84");
        S.T<int>(dirs[i], "A85");
        S.T<int>(dirs[i], "A86");
        S.T<int>(dirs[i], "A87");
        S.T<int>(dirs[i], "A88");
        S.T<int>(dirs[i], "A89");
        S.T<int>(dirs[i], "A90");
        S.T<int>(dirs[i], "A91");
        S.T<int>(dirs[i], "A92");
        S.T<int>(dirs[i], "A93");
        S.T<int>(dirs[i], "A94");
        S.T<int>(dirs[i], "A95");
        S.T<int>(dirs[i], "A96");
        S.T<int>(dirs[i], "A97");
        S.T<int>(dirs[i], "A98");
        S.T<int>(dirs[i], "A99");
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
    ObjectBackStore backstore;
    ObjectStore S = backstore.store();
    for (int i = 0; i < N; i++) for(int j = 0; j < M; j++) lookup1000(S("test/", i%2, "/", j%5, "/"));
    std::cout << 1000*N*M << std::endl;
    return 0;
}
