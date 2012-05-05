#include <stdlib.h>
#include <iostream>

#ifdef HAVE_CERN_ROOT_SYSTEM

#include <TFile.h>

#include <a4/storable.h>
using a4::store::Storable;

#include <a4/root_object_store.h>
using a4::store::RootObjectStore;
using a4::store::ObjectStore;
using a4::store::RTH1;

class Test : public Storable {
public:
    virtual Storable& operator+=(const Storable &other) { abort(); }
    virtual Storable&& operator+(const Storable &other) { abort(); }
    virtual Storable& operator*=(const double&) { abort(); }
    virtual Storable&& clone_storable() { abort(); }
    
    void yo() {
        std::cout << "Yo! this = " << this << std::endl;
    }
};


int main(int argc, char* argv[]) {    
    RootObjectStore backstore;
    ObjectStore S = backstore.store();
    
    for (int i = 0; i < 100; i++ ) {
        S.T<RTH1>("test/test/blah")(100, 0., 100.).fill(100);
    }
    
    TFile f("test.root", "recreate");
    backstore.write();
    return 0;
}

#else // not HAVE_CERN_ROOT_SYSTEM

int main(int argc, char* argv[]) {
    std::cout << "Not configured with ROOT available" << std::endl;
    return 0;
}

#endif // HAVE_CERN_ROOT_SYSTEM
