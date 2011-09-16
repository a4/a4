#ifndef _A4_OBJECT_STORE_H_
#define _A4_OBJECT_STORE_H_

#include <a4/hash_lookup.h>

class BackObjectStore;


class ObjectStore {
    public:
        template <class C, typename ...Args> C & T(const Args & ...args);
        template <class C, typename ...Args> C & slow(const Args & ...args);
        template <typename ...Args> ObjectStore operator()(const Args & ...args);
    protected:
        ObjectStore(hash_lookup * hl, BackObjectStore* bs) : hl(hl), backstore(bs) {};
        BackObjectStore * backstore;
        hash_lookup * hl;
        friend class BackObjectStore;
};

template <class Base>
class CheckedObjectStore {
    public:
        template <class Cls, typename ...Args> Cls & T(const Args & ...args);
        template <class C, typename ...Args> C & slow(const Args & ...args);
        template <typename ...Args> CheckedObjectStore operator()(const Args & ...args);
    protected:
        CheckedObjectStore(hash_lookup * hl, BackObjectStore* bs) : hl(hl), backstore(bs) {};
        BackObjectStore * backstore;
        hash_lookup * hl;
        friend class BackObjectStore;
};

class BackObjectStore {
    public:
        BackObjectStore();
        ~BackObjectStore();
        ObjectStore store();
        template <class Base>
        CheckedObjectStore<Base> checked_store();
        template <class C, typename ...Args> C * find(const Args & ...args);
    private:
        hash_lookup hl;
        std::map<std::string, void*> _store;
};

#include <a4/object_store_impl.h>

#endif
