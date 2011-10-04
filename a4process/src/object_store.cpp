#include "a4/object_store.h"

namespace a4{ namespace process{
    ObjectBackStore::ObjectBackStore() {};

    ObjectBackStore::~ObjectBackStore() {};

    ObjectStore ObjectBackStore::store() { return ObjectStore(&hl, this); };

    void ObjectBackStore::to_stream(a4::io::A4OutputStream &outs) const {
        foreach(std::map<std::string, void*>::const_iterator i, _store) {
            A4Key k;
            k.set_name(i->first);

            
            outs.write(k);
            

        } 
        
        
    }
    void ObjectBackStore::from_stream(a4::io::A4InputStream & ins);

};};
