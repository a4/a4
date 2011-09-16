#ifndef _A4_OBJECT_STORE_IMPL_H_
#define _A4_OBJECT_STORE_IMPL_H_

template <class C, typename ...Args> C & ObjectStore::T(const Args & ...args) {
    void * & res = hl->lookup(args...);
    if (res) return *(static_cast<C*>(res));
    res = backstore->find<C>(hl->get_path(), args...);
    return *(static_cast<C*>(res));
};

template <class C, typename ...Args> C & ObjectStore::slow(const Args & ...args) {
    void * &res = backstore->find<C>(hl->get_path(), args...);
    return *(static_cast<C*>(res));
};

template <typename ...Args> ObjectStore ObjectStore::operator()(const Args & ...args) {
    return ObjectStore(hl->subhash(args...), backstore);
}

template <class Base>
template <class Cls, typename ...Args> Cls & CheckedObjectStore<Base>::T(const Args & ...args) {
    Base * b = static_cast<Base*>(&ObjectStore::T(args...));
    return *dynamic_cast<Cls*>(b);
}

template <class Base>
template <class C, typename ...Args> C & CheckedObjectStore<Base>::slow(const Args & ...args) {
    Base * b = static_cast<Base*>(backstore->find<C>(hl->get_path(), args...));
    return *dynamic_cast<C*>(b);
};

template <class Base>
template <typename ...Args> CheckedObjectStore<Base> CheckedObjectStore<Base>::operator()(const Args & ...args) {
    return CheckedObjectStore(hl->subhash(args...), backstore);
}

BackObjectStore::BackObjectStore() {};
BackObjectStore::~BackObjectStore() {};

ObjectStore BackObjectStore::store() { return ObjectStore(&hl, this); };

template <class Base>
CheckedObjectStore<Base> BackObjectStore::checked_store() { return CheckedObjectStore<Base>(&hl, this); };

template <class C, typename ...Args> C * BackObjectStore::find(const Args & ...args) {
    std::string name = str_cat(args...);
    void * & res = _store[name];
    if (res) return static_cast<C*>(res);
    C* p = new C();
    res = p;
    return p;
};


#endif
