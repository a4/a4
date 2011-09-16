#ifndef _A4_OBJECTSTORE_IMPL_H_
#define _A4_OBJECTSTORE_IMPL_H_

#include <sstream>

// Retrieve a pointer to a T from the store with no checks, using its full name
template <class STORE>
template <class T> 
boost::shared_ptr<T> ObjectStore<STORE>::get(const std::string &name) {
    boost::shared_ptr<STORE> & res = main_storage[name];
    if (!res) res.reset(new T());
    return boost::static_pointer_cast<T>(res);
}

// Map "name" to a pointer to T. Passes ownership of T to the Store.
template <class STORE>
void ObjectStore<STORE>::set(const std::string &name, STORE* newly_created) {
    boost::shared_ptr<STORE> p(newly_created);
    main_storage[name] = p;
}

// Map "name" to a pointer
template <class STORE>
void ObjectStore<STORE>::set(const std::string &name, boost::shared_ptr<STORE> obj) {
    main_storage[name] = obj;
}

// Retrieve a pointer to T from the store with type checked
template <class STORE>
template <class T> 
boost::shared_ptr<T> ObjectStore<STORE>::get_checked(const std::string &name) const {
    typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator obj;
    obj = main_storage.find(name);
    if (obj == main_storage.end()) return boost::shared_ptr<T>();
    return boost::dynamic_pointer_cast<T>(obj->second);
}

// List all names in the Store
template <class STORE>
std::vector<std::string> ObjectStore<STORE>::list() const {
    std::vector<std::string> res;
    for (typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator it = main_storage.begin(), end = main_storage.end(); it != end; it++) {
        res.push_back(it->first);
    }
    return res;
}

// List all names of a given class in the store
template <class STORE>
template <class T>
std::vector<std::string> ObjectStore<STORE>::list() const {
    std::vector<std::string> res;
    for (typename std::map<std::string, boost::shared_ptr<STORE> >::const_iterator it = main_storage.begin(), end = main_storage.end(); it != end; it++) {
        if ((boost::dynamic_pointer_cast<T>(it->second)).get())
            res.push_back(it->first);
    }
    return res;
}

template <class STORE>
template <class T, typename... Args>
T & ObjectStore<STORE>::get_fmt(const char * name_fmt, const Args& ...args) {
    void * & res = fast_lookup.lookup(args...);
    if (res != NULL) return *(static_cast<T*>(res));
    boost::shared_ptr<T> t = get<T>(str_printf(args...));
    res = t.get();
    return *t.get();
}

template <class STORE>
template <class T, typename... Args>
T & ObjectStore<STORE>::get_cat(const Args& ...args) {
    void * & res = fast_lookup.lookup(args...);
    if (res != NULL) return *(static_cast<T*>(res));
    boost::shared_ptr<T> t = get<T>(str_cat(args...));
    res = t.get();
    return *t.get();
}

template <class STORE>
template <class T, typename... Args>
T & ObjectStore<STORE>::get_hint(uint32_t hint, const Args& ...args) {
    void * & res = fast_lookup.lookup(hint, args...);
    if (res != NULL) return *(static_cast<T*>(res));
    boost::shared_ptr<T> t = get<T>(str_cat(args...));
    res = t.get();
    return *t.get();
}

#endif
