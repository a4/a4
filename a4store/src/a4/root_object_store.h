#ifndef _A4_STORE_ROOT_OBJECT_STORE_H_
#define _A4_STORE_ROOT_OBJECT_STORE_H_

#ifdef HAVE_CERN_ROOT_SYSTEM

#include <assert.h>

#include <string>

#include <TFile.h>
#include <TDirectory.h>
#include <TH1D.h>

#include <a4/storable.h>
#include <a4/object_store.h>

namespace a4 {
namespace store {

class RootStorable : public Storable {
public:
    virtual TObject* get_tobject() = 0;
};


// TODO: Figure out how different dimensionalities of histograms will work

template<class ROOT_TYPE>
class SpecificRootStorable : public RootStorable {
public:
    typedef SpecificRootStorable<ROOT_TYPE> This;

    SpecificRootStorable() : _initializations_remaining(1) {}


#ifdef A4STORE_STANDALONE
            // TODO: implement alternative storage mechanisms
#else
    virtual shared<const google::protobuf::Message> as_message() { FATAL("Can't serialize root objects to protobuf"); }
    virtual void construct_from(const google::protobuf::Message&) { FATAL("Can't build root obects from protobuf"); }
    virtual void construct_from(shared<google::protobuf::Message> m) { construct_from(*m); }
#endif // ifdef A4STORE_STANDALONE

    virtual Storable& operator+=(const Storable &other) { abort(); }
    virtual Storable&& operator+(const Storable &other) { abort(); }
    virtual Storable& operator*=(const double&) { abort(); }
    virtual Storable&& clone_storable() { abort(); }
    
    ROOT_TYPE root_object;
    
    TObject* get_tobject() { return &root_object; }
    
    void constructor(const char* title) {
        _initializations_remaining++;
        root_object.SetTitle(title);
    }
    void constructor(const uint32_t bins, const double min, const double max, const char* label="") {
        root_object.SetBins(bins, min, max);
        root_object.Reset();
    }
    void constructor(const std::vector<double>& bins, const char* label="") {
        abort();
    }
    
    template <typename... Args> This& operator()(const Args&... args) {
        if (_initializations_remaining != 0) {
            _initializations_remaining--;
            static_cast<This*>(this)->constructor(args...);
        }
        return *static_cast<This*>(this);
    }
    
    int _initializations_remaining;
    bool _initialized() const { return _initializations_remaining == 0; }
    
    void fill(double v, double w=1) {
        root_object.Fill(v, w);
    }
};

typedef SpecificRootStorable<TH1D> RTH1;

class RootObjectStore : public ObjectBackStore {
public:

    static TDirectory* mkdirs(TDirectory* start, const std::string& file) {
        if (!start->GetDirectory(file.c_str()))
            start->mkdir(file.c_str());
        TDirectory* d = start->GetDirectory(file.c_str());
        d->cd();
        return d;
    }
    
    void write() {
        TDirectory* destination = gDirectory;
        
        assert(destination->IsWritable());
        
        for (std::map<std::string, shared<Storable> >::iterator i = _store->begin(); 
            i != _store->end(); i++) {

            RootStorable* object = dynamic_cast<RootStorable*>(i->second.get());

            if (!object) {
                std::cout << "Non-RootStorable object: " << i->first << std::endl;
                continue;
            }
            
            // TODO: apply basename/dirname, SetName();

            TDirectory* d = mkdirs(destination, i->first);
            d->WriteTObject(object->get_tobject());

            //std::cout << "Would have written storable " << i->first << " ptr = " << i->second.get() << std::endl;
        }
        
    }
};
    

}
}
    
#endif // HAVE_CERN_ROOT_SYSTEM

#endif // _A4_STORE_ROOT_OBJECT_STORE_H_

