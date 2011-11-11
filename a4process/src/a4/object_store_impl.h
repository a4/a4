#ifndef _A4_OBJECT_STORE_IMPL_H_
#define _A4_OBJECT_STORE_IMPL_H_

#include <boost/type_traits.hpp>

#include <a4/types.h>

namespace a4{ namespace process{
    using a4::process::Storable;

    template <class C, typename ...Args> C & ObjectStore::T(const Args & ...args) {
        // First, get a reference(!) to a void pointer from the fast lookup table
        void * & res = hl->lookup(args...);
        // if these args... are found, return the result as a C*
        if (res) return *static_cast<C*>(res);
        // ..otherwise _set_ the reference to the backstore result.
        // This updates the fast lookup table.
        res = backstore->find<C>(hl->get_path(), args...);
        // Return the result from the backstore.
        return *static_cast<C*>(res);
    };

    template <class C, typename ...Args> C & ObjectStore::T_slow(const Args & ...args) {
        return *static_cast<C*>(backstore->find<C>(hl->get_path(), args...));
    };

    template <class C> shared<C> ObjectStore::get_slow(const std::string & name) {
        return backstore->get<C>(hl->get_path() + name);
    };

    inline void ObjectStore::set_slow(const std::string & name, shared<Storable> obj) {
        backstore->set(hl->get_path() + name, obj);
    };

    template <typename ...Args> ObjectStore ObjectStore::operator()(const Args & ...args) {
        return ObjectStore(hl->subhash(args...), backstore);
    }

    template <class C, typename ...Args> C * ObjectStore::find(const Args & ...args) {
        BOOST_STATIC_ASSERT_MSG((boost::is_convertible<C*, Storable*>::value), "You can only store objects that implement the Storable interface into the ObjectStore!");
        return dynamic_cast<C*>(static_cast<Storable*>(&T<C>(args...)));
    }

    template <class C, typename ...Args> C * ObjectStore::find_slow(const Args & ...args) {
        BOOST_STATIC_ASSERT_MSG((boost::is_convertible<C*, Storable*>::value), "You can only store objects that implement the Storable interface into the ObjectStore!");
        return dynamic_cast<C*>(static_cast<Storable*>(backstore->find<C>(hl->get_path(), args...)));
    };

    template <class C, typename ...Args> C * ObjectBackStore::find(const Args & ...args) {
        BOOST_STATIC_ASSERT_MSG((boost::is_convertible<C*, Storable*>::value), "You can only store objects that implement the Storable interface into the ObjectStore!");
        std::string name = str_cat(args...);
        shared<Storable> & res = (*_store)[name];
        if (res) return static_cast<C*>(res.get());
        res.reset(new C());
        return static_cast<C*>(res.get());
    };

    template <class C> std::vector<std::string> ObjectBackStore::list() const {
        std::vector<std::string> res;
        std::map<std::string, shared<Storable>>::const_iterator i;
        for (i = _store->begin(); i != _store->end(); i++) {
            if(dynamic_pointer_cast<C>(i->second)) res.push_back(i->first);
        }
        return res;
    };

    template <class C> shared<C> ObjectBackStore::get(std::string s) const {
        std::map<std::string, shared<Storable>>::const_iterator res;
        res = _store->find(s);
        if (res!=_store->end()) return dynamic_pointer_cast<C>(res->second);
        return shared<C>();
    };

};};

#endif
