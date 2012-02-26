#include <iostream>
#include <string>
#include <cassert>

#include <a4/object_store.h>
#include <a4/object_store_impl.h>

using namespace std;
using namespace a4::process;

std::vector<std::string> items;

class myhist : public Storable {
    public:
        virtual shared<const google::protobuf::Message> as_message() {
            return shared<google::protobuf::Message>();
        }
        virtual void set_message(const google::protobuf::Message&) {}
        virtual void set_message(shared<google::protobuf::Message>) {}
        void construct_from(const google::protobuf::Message&) {}
        void construct_from(std::shared_ptr<google::protobuf::Message>) {}
        Storable&  operator+=(const a4::process::Storable&) { return *this; }
        Storable&  operator*=(const double&) { return *this; }
        Storable&& clone_storable() { return std::move(myhist(*this)); }
};

template <typename... Args>
void lookup1000(ObjectStore S) {
    const char* dirs[] = {"alpha/", "beta/", "gamma/", "delta/", "epsilon/", "phi/", "chi/", "xi/", "tau/", "omikron/"};
    for (int i = 0; i < 10; i++) {
        S.T<myhist>(dirs[i], "A00");
        S.T<myhist>(dirs[i], "A01");
        S.T<myhist>(dirs[i], "A02");
        S.T<myhist>(dirs[i], "A03");
        S.T<myhist>(dirs[i], "A04");
        S.T<myhist>(dirs[i], "A05");
        S.T<myhist>(dirs[i], "A06");
        S.T<myhist>(dirs[i], "A07");
        S.T<myhist>(dirs[i], "A08");
        S.T<myhist>(dirs[i], "A09");
        S.T<myhist>(dirs[i], "A10");
        S.T<myhist>(dirs[i], "A11");
        S.T<myhist>(dirs[i], "A12");
        S.T<myhist>(dirs[i], "A13");
        S.T<myhist>(dirs[i], "A14");
        S.T<myhist>(dirs[i], "A15");
        S.T<myhist>(dirs[i], "A16");
        S.T<myhist>(dirs[i], "A17");
        S.T<myhist>(dirs[i], "A18");
        S.T<myhist>(dirs[i], "A19");
        S.T<myhist>(dirs[i], "A20");
        S.T<myhist>(dirs[i], "A21");
        S.T<myhist>(dirs[i], "A22");
        S.T<myhist>(dirs[i], "A23");
        S.T<myhist>(dirs[i], "A24");
        S.T<myhist>(dirs[i], "A25");
        S.T<myhist>(dirs[i], "A26");
        S.T<myhist>(dirs[i], "A27");
        S.T<myhist>(dirs[i], "A28");
        S.T<myhist>(dirs[i], "A29");
        S.T<myhist>(dirs[i], "A30");
        S.T<myhist>(dirs[i], "A31");
        S.T<myhist>(dirs[i], "A32");
        S.T<myhist>(dirs[i], "A33");
        S.T<myhist>(dirs[i], "A34");
        S.T<myhist>(dirs[i], "A35");
        S.T<myhist>(dirs[i], "A36");
        S.T<myhist>(dirs[i], "A37");
        S.T<myhist>(dirs[i], "A38");
        S.T<myhist>(dirs[i], "A39");
        S.T<myhist>(dirs[i], "A40");
        S.T<myhist>(dirs[i], "A41");
        S.T<myhist>(dirs[i], "A42");
        S.T<myhist>(dirs[i], "A43");
        S.T<myhist>(dirs[i], "A44");
        S.T<myhist>(dirs[i], "A45");
        S.T<myhist>(dirs[i], "A46");
        S.T<myhist>(dirs[i], "A47");
        S.T<myhist>(dirs[i], "A48");
        S.T<myhist>(dirs[i], "A49");
        S.T<myhist>(dirs[i], "A50");
        S.T<myhist>(dirs[i], "A51");
        S.T<myhist>(dirs[i], "A52");
        S.T<myhist>(dirs[i], "A53");
        S.T<myhist>(dirs[i], "A54");
        S.T<myhist>(dirs[i], "A55");
        S.T<myhist>(dirs[i], "A56");
        S.T<myhist>(dirs[i], "A57");
        S.T<myhist>(dirs[i], "A58");
        S.T<myhist>(dirs[i], "A59");
        S.T<myhist>(dirs[i], "A60");
        S.T<myhist>(dirs[i], "A61");
        S.T<myhist>(dirs[i], "A62");
        S.T<myhist>(dirs[i], "A63");
        S.T<myhist>(dirs[i], "A64");
        S.T<myhist>(dirs[i], "A65");
        S.T<myhist>(dirs[i], "A66");
        S.T<myhist>(dirs[i], "A67");
        S.T<myhist>(dirs[i], "A68");
        S.T<myhist>(dirs[i], "A69");
        S.T<myhist>(dirs[i], "A70");
        S.T<myhist>(dirs[i], "A71");
        S.T<myhist>(dirs[i], "A72");
        S.T<myhist>(dirs[i], "A73");
        S.T<myhist>(dirs[i], "A74");
        S.T<myhist>(dirs[i], "A75");
        S.T<myhist>(dirs[i], "A76");
        S.T<myhist>(dirs[i], "A77");
        S.T<myhist>(dirs[i], "A78");
        S.T<myhist>(dirs[i], "A79");
        S.T<myhist>(dirs[i], "A80");
        S.T<myhist>(dirs[i], "A81");
        S.T<myhist>(dirs[i], "A82");
        S.T<myhist>(dirs[i], "A83");
        S.T<myhist>(dirs[i], "A84");
        S.T<myhist>(dirs[i], "A85");
        S.T<myhist>(dirs[i], "A86");
        S.T<myhist>(dirs[i], "A87");
        S.T<myhist>(dirs[i], "A88");
        S.T<myhist>(dirs[i], "A89");
        S.T<myhist>(dirs[i], "A90");
        S.T<myhist>(dirs[i], "A91");
        S.T<myhist>(dirs[i], "A92");
        S.T<myhist>(dirs[i], "A93");
        S.T<myhist>(dirs[i], "A94");
        S.T<myhist>(dirs[i], "A95");
        S.T<myhist>(dirs[i], "A96");
        S.T<myhist>(dirs[i], "A97");
        S.T<myhist>(dirs[i], "A98");
        S.T<myhist>(dirs[i], "A99");
    }
}

template <typename... Args>
void check_set(hash_lookup* h, const Args& ...args) {
    string*& res = (string*&)h->lookup(args...);
    assert(res != NULL);
    assert(*res == str_cat(args...));
}

template <typename... Args>
void test_check_set(hash_lookup* h, const Args& ...args) {
    test_set(h, args...);
    check_set(h, args...);
}

int main(int argv, char ** argc) {
    const int N = 1000;
    const int M = 100;
    ObjectBackStore backstore;
    ObjectStore S = backstore.store();
    for (int i = 0; i < N; i++) for(int j = 0; j < M; j++) lookup1000(S("test/", i%2, "/", j%5, "/"));
    std::cout << 1000*N*M << std::endl;
    return 0;
}
