#ifndef _A4_OBJECT_STORE_H_
#define _A4_OBJECT_STORE_H_

#include <a4/hash_lookup.h>

class ObjectBackStore;

extern void compile_error(std::string);

class ObjectStore {
    public:
        ObjectStore() {};
        template <class C, typename ...Args> C & T(const Args & ...args);
        template <typename ...Args> void T(const Args & ...args) { compile_error("Call T with a template parameter: S.T<H1>(\"my histogram\")"); };
        template <class C, typename ...Args> C & slow(const Args & ...args);
        template <typename ...Args> ObjectStore operator()(const Args & ...args);
    protected:
        ObjectStore(hash_lookup * hl, ObjectBackStore* bs) : hl(hl), backstore(bs) {};
        ObjectBackStore * backstore;
        hash_lookup * hl;
        friend class ObjectBackStore;
};

template <class Base>
class CheckedObjectStore {
    public:
        template <class Cls, typename ...Args> Cls & T(const Args & ...args);
        template <class C, typename ...Args> C & slow(const Args & ...args);
        template <typename ...Args> CheckedObjectStore operator()(const Args & ...args);
    protected:
        CheckedObjectStore(hash_lookup * hl, ObjectBackStore* bs) : hl(hl), backstore(bs) {};
        ObjectBackStore * backstore;
        hash_lookup * hl;
        friend class ObjectBackStore;
};

class ObjectBackStore {
    public:
        ObjectBackStore();
        ~ObjectBackStore();
        ObjectStore store();
        template <class Base>
        CheckedObjectStore<Base> checked_store();
        template <class C, typename ...Args> C * find(const Args & ...args);
        template <typename ...Args> ObjectStore operator()(const Args & ...args) { return store()(args...); };
    private:
        hash_lookup hl;
        std::map<std::string, void*> _store;
};

#include <a4/object_store_impl.h>

#endif
