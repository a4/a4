#include <iostream>
#include <string>
#include <cassert>

#include <a4/object_store.h>
#include <a4/storable.h>
#include <a4/output_stream.h>
#include <a4/input_stream.h>
#include <a4/proto/process/A4Key.pb.h>

using namespace std;
using namespace a4::process;
using namespace a4::io;

std::vector<std::string> items;

class myhist : public StorableAs<myhist, TestHisto> {
    public:
        myhist & operator()(const double & d) { 
            if (_initialized) return *this;
            j = d;
            pb.reset(new TestHisto());
            _initialized = true;
            return *this;
        }
        void to_pb() { assert(pb); pb->set_data(j); };
        void from_pb() { assert(pb); j = pb->data(); };
        void work() { j += 1.2; };
        double j;
};

template <typename... Args>
void lookup1000(ObjectStore S) {
    const char * dirs[] = {"alpha/", "beta/", "gamma/", "delta/", "epsilon/", "phi/", "chi/", "xi/", "tau/", "omikron/"};
    for (int i = 0; i < 10; i++) {
        S.T<myhist>(dirs[i], "A00")(0.1).work();
        S.T<myhist>(dirs[i], "A01")(0.1).work();
        S.T<myhist>(dirs[i], "A02")(0.1).work();
        S.T<myhist>(dirs[i], "A03")(0.1).work();
        S.T<myhist>(dirs[i], "A04")(0.1).work();
        S.T<myhist>(dirs[i], "A05")(0.1).work();
        S.T<myhist>(dirs[i], "A06")(0.1).work();
        S.T<myhist>(dirs[i], "A07")(0.1).work();
        S.T<myhist>(dirs[i], "A08")(0.1).work();
        S.T<myhist>(dirs[i], "A09")(0.1).work();
        S.T<myhist>(dirs[i], "A10")(0.1).work();
        S.T<myhist>(dirs[i], "A11")(0.1).work();
        S.T<myhist>(dirs[i], "A12")(0.1).work();
        S.T<myhist>(dirs[i], "A13")(0.1).work();
        S.T<myhist>(dirs[i], "A14")(0.1).work();
        S.T<myhist>(dirs[i], "A15")(0.1).work();
        S.T<myhist>(dirs[i], "A16")(0.1).work();
        S.T<myhist>(dirs[i], "A17")(0.1).work();
        S.T<myhist>(dirs[i], "A18")(0.1).work();
        S.T<myhist>(dirs[i], "A19")(0.1).work();
        S.T<myhist>(dirs[i], "A20")(0.1).work();
        S.T<myhist>(dirs[i], "A21")(0.1).work();
        S.T<myhist>(dirs[i], "A22")(0.1).work();
        S.T<myhist>(dirs[i], "A23")(0.1).work();
        S.T<myhist>(dirs[i], "A24")(0.1).work();
        S.T<myhist>(dirs[i], "A25")(0.1).work();
        S.T<myhist>(dirs[i], "A26")(0.1).work();
        S.T<myhist>(dirs[i], "A27")(0.1).work();
        S.T<myhist>(dirs[i], "A28")(0.1).work();
        S.T<myhist>(dirs[i], "A29")(0.1).work();
        S.T<myhist>(dirs[i], "A30")(0.1).work();
        S.T<myhist>(dirs[i], "A31")(0.1).work();
        S.T<myhist>(dirs[i], "A32")(0.1).work();
        S.T<myhist>(dirs[i], "A33")(0.1).work();
        S.T<myhist>(dirs[i], "A34")(0.1).work();
        S.T<myhist>(dirs[i], "A35")(0.1).work();
        S.T<myhist>(dirs[i], "A36")(0.1).work();
        S.T<myhist>(dirs[i], "A37")(0.1).work();
        S.T<myhist>(dirs[i], "A38")(0.1).work();
        S.T<myhist>(dirs[i], "A39")(0.1).work();
        S.T<myhist>(dirs[i], "A40")(0.1).work();
        S.T<myhist>(dirs[i], "A41")(0.1).work();
        S.T<myhist>(dirs[i], "A42")(0.1).work();
        S.T<myhist>(dirs[i], "A43")(0.1).work();
        S.T<myhist>(dirs[i], "A44")(0.1).work();
        S.T<myhist>(dirs[i], "A45")(0.1).work();
        S.T<myhist>(dirs[i], "A46")(0.1).work();
        S.T<myhist>(dirs[i], "A47")(0.1).work();
        S.T<myhist>(dirs[i], "A48")(0.1).work();
        S.T<myhist>(dirs[i], "A49")(0.1).work();
        S.T<myhist>(dirs[i], "A50")(0.1).work();
        S.T<myhist>(dirs[i], "A51")(0.1).work();
        S.T<myhist>(dirs[i], "A52")(0.1).work();
        S.T<myhist>(dirs[i], "A53")(0.1).work();
        S.T<myhist>(dirs[i], "A54")(0.1).work();
        S.T<myhist>(dirs[i], "A55")(0.1).work();
        S.T<myhist>(dirs[i], "A56")(0.1).work();
        S.T<myhist>(dirs[i], "A57")(0.1).work();
        S.T<myhist>(dirs[i], "A58")(0.1).work();
        S.T<myhist>(dirs[i], "A59")(0.1).work();
        S.T<myhist>(dirs[i], "A60")(0.1).work();
        S.T<myhist>(dirs[i], "A61")(0.1).work();
        S.T<myhist>(dirs[i], "A62")(0.1).work();
        S.T<myhist>(dirs[i], "A63")(0.1).work();
        S.T<myhist>(dirs[i], "A64")(0.1).work();
        S.T<myhist>(dirs[i], "A65")(0.1).work();
        S.T<myhist>(dirs[i], "A66")(0.1).work();
        S.T<myhist>(dirs[i], "A67")(0.1).work();
        S.T<myhist>(dirs[i], "A68")(0.1).work();
        S.T<myhist>(dirs[i], "A69")(0.1).work();
        S.T<myhist>(dirs[i], "A70")(0.1).work();
        S.T<myhist>(dirs[i], "A71")(0.1).work();
        S.T<myhist>(dirs[i], "A72")(0.1).work();
        S.T<myhist>(dirs[i], "A73")(0.1).work();
        S.T<myhist>(dirs[i], "A74")(0.1).work();
        S.T<myhist>(dirs[i], "A75")(0.1).work();
        S.T<myhist>(dirs[i], "A76")(0.1).work();
        S.T<myhist>(dirs[i], "A77")(0.1).work();
        S.T<myhist>(dirs[i], "A78")(0.1).work();
        S.T<myhist>(dirs[i], "A79")(0.1).work();
        S.T<myhist>(dirs[i], "A80")(0.1).work();
        S.T<myhist>(dirs[i], "A81")(0.1).work();
        S.T<myhist>(dirs[i], "A82")(0.1).work();
        S.T<myhist>(dirs[i], "A83")(0.1).work();
        S.T<myhist>(dirs[i], "A84")(0.1).work();
        S.T<myhist>(dirs[i], "A85")(0.1).work();
        S.T<myhist>(dirs[i], "A86")(0.1).work();
        S.T<myhist>(dirs[i], "A87")(0.1).work();
        S.T<myhist>(dirs[i], "A88")(0.1).work();
        S.T<myhist>(dirs[i], "A89")(0.1).work();
        S.T<myhist>(dirs[i], "A90")(0.1).work();
        S.T<myhist>(dirs[i], "A91")(0.1).work();
        S.T<myhist>(dirs[i], "A92")(0.1).work();
        S.T<myhist>(dirs[i], "A93")(0.1).work();
        S.T<myhist>(dirs[i], "A94")(0.1).work();
        S.T<myhist>(dirs[i], "A95")(0.1).work();
        S.T<myhist>(dirs[i], "A96")(0.1).work();
        S.T<myhist>(dirs[i], "A97")(0.1).work();
        S.T<myhist>(dirs[i], "A98")(0.1).work();
        S.T<myhist>(dirs[i], "A99")(0.1).work();
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
    ObjectStore S_test = S("test/");

    assert(S_test.find<myhist>("goo").j == 0);

    for (int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            ObjectStore subdir = S("test/", i%2, "/", j%5, "/");
            lookup1000(subdir);
        }
    }
    std::cout << 1000*N*M << std::endl;
    {
        A4OutputStream out("test_storable.a4", "Store Test");
        out.content_cls<A4Key>().metadata_cls<TestHistoMetaData>();
        backstore.to_stream(out);
    }
    ObjectBackStore inbs;
    {
        A4InputStream in("test_storable.a4");
        in.new_metadata();
        inbs.from_stream(in);
    }
    
    return 0;
}
