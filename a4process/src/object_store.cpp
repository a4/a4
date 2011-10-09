#include <a4/object_store.h>
#include <a4/proto/process/A4Key.pb.h>
#include <a4/register.h>
#include <a4/message.h>
#include <a4/output_stream.h>
#include <a4/input_stream.h>
#include <iostream>

A4RegisterClass(a4::process::A4Key);
A4RegisterClass(a4::process::TestHisto);
A4RegisterClass(a4::process::TestHistoMetaData);

namespace a4{ namespace process{

    ObjectBackStore::ObjectBackStore() {
        hl.reset(new hash_lookup());
        _store.reset(new std::map<std::string, shared<Storable>>());
    };

    ObjectBackStore::~ObjectBackStore() {};

    ObjectStore ObjectBackStore::store() { return ObjectStore(hl.get(), this); };

    void ObjectBackStore::to_stream(a4::io::A4OutputStream &outs) const {
        std::map<std::string, shared<Storable>>::const_iterator i;
        for(i = _store->begin(); i != _store->end(); i++) {
            A4Key k;
            k.set_name(i->first);
            shared<const google::protobuf::Message> msg(i->second->as_message());
            outs.write(k);
            if (!msg) std::cerr << "HOAOAOA" << std::endl;
            assert(msg);
            outs.write(*msg);
        } 
    };

    void ObjectBackStore::from_stream(a4::io::A4InputStream & ins) {
        while (! ins.new_metadata() && ins.good()) {
            a4::io::A4Message key = ins.next();
            if (!key.is<A4Key>()) continue;
            a4::io::A4Message val = ins.next();
            std::string name = key.as<A4Key>()->name();
            (*_store)[name] = message_to_storable(val);
        }
    
    };

    std::vector<std::string> ObjectBackStore::list() const {
        std::vector<std::string> res;
        std::map<std::string, shared<Storable>>::const_iterator i;
        for (i = _store->begin(); i != _store->end(); i++) {
            res.push_back(i->first);
        }
        return res;
    };

};};
