// This file targets gcc 4.3. 
// Please no auto

#include <iostream>
#include <vector>

//#include <google/protobuf/message.h>

#ifndef A4STORE_STANDALONE
#include <a4/register.h>
#include <a4/message.h>
#include <a4/output_stream.h>
#include <a4/input_stream.h>


#include <a4/store/A4Key.pb.h>
A4RegisterClass(a4::store::A4Key);
#endif

#include <a4/object_store.h>

namespace a4 {
namespace store {

    ObjectBackStore::ObjectBackStore() {
        hl.reset(new hash_lookup());
        _store.reset(new std::map<std::string, shared<Storable>>());
    }

    ObjectBackStore::~ObjectBackStore() {}

    ObjectStore ObjectBackStore::store() { 
        return ObjectStore(hl.get(), this, 1.0); 
    }

#ifndef A4STORE_STANDALONE
    void ObjectBackStore::to_stream(a4::io::OutputStream& outs) const {
        for (auto i = _store->begin(); i != _store->end(); i++) {
            A4Key k;
            k.set_name(i->first);
            shared<const google::protobuf::Message> msg(i->second->as_message());
            outs.write(k);
            
            if (!msg) 
                FATAL("NULL message encountered");
                
            outs.write(*msg);
        } 
    }

    void ObjectBackStore::from_stream(a4::io::InputStream& ins) {
        while (!ins.new_metadata() && ins.good()) {
            shared<a4::io::A4Message> key = ins.next();
            if (!key) return;
            if (!key->is<A4Key>()) 
                continue;
            shared<a4::io::A4Message> val = ins.next();
            std::string name = key->as<A4Key>()->name();
            (*_store)[name] = message_to_storable(val);
        }
    
    }
#endif

    std::vector<std::string> ObjectBackStore::list() const {
        std::vector<std::string> res;
        for (std::map<std::string, shared<Storable>>::const_iterator i = _store->begin();
             i != _store->end(); i++) {
            res.push_back(i->first);
        }
        return res;
    }

};};
