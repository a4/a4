#ifndef _A4_OBJECT_STORE_IMPL_H_
#define _A4_OBJECT_STORE_IMPL_H_

#include <boost/type_traits.hpp>

#include <a4/types.h>


namespace a4 {
namespace process {

    using a4::process::Storable;

    template <class C, typename ...Args> 
    C& ObjectStore::T(const Args& ...args) {
        // First, get a reference(!) to a void pointer from the fast lookup table
        void*& resp = hl->lookup(args...);
        C* res = static_cast<C*>(resp);
        // if these args... are found, return the result as a C*
        if (res) {
            res->weight(current_weight);
            return *res;
        }
        // ..otherwise _set_ the reference to the backstore result.
        // This updates the fast lookup table.
        resp = res = backstore->find<C>(hl->get_path(), args...);
        // Return the result from the backstore.
        res->weight(current_weight);
        return *res;
    }
    
    template <class C, typename ...Args> 
    C& ObjectStore::T_slow(const Args& ...args) {
        C* res = static_cast<C*>(backstore->find<C>(hl->get_path(), args...));
        res->weight(current_weight);
        return *res;
    }

    template <class C> 
    shared<C> ObjectStore::get_slow(const std::string& name) const {
        auto res = backstore->get<C>(hl->get_path() + name);
        if (res) static_cast<Storable*>(res.get())->weight(current_weight);
        return res;
    }

    inline void ObjectStore::set_slow(const std::string& name, shared<Storable> obj) {
        backstore->set(hl->get_path() + name, obj);
    }

    template <typename ...Args> 
    ObjectStore ObjectStore::operator()(const Args& ...args) const {
        return ObjectStore(hl->subhash(args...), backstore, current_weight);
    }

    template <class C, typename ...Args> 
    C* ObjectStore::find(const Args& ...args) {
        BOOST_STATIC_ASSERT_MSG((boost::is_convertible<C*, Storable*>::value), 
            "You can only store objects that implement the Storable interface into the ObjectStore!");
        auto res = dynamic_cast<C*>(static_cast<Storable*>(&T<C>(args...)));
        if (res)
            static_cast<Storable*>(res)->weight(current_weight);
        return res;
    }

    template <class C, typename ...Args> 
    C* ObjectStore::find_slow(const Args& ...args) {
        BOOST_STATIC_ASSERT_MSG((boost::is_convertible<C*, Storable*>::value), 
            "You can only store objects that implement the Storable interface into the ObjectStore!");
        auto res = dynamic_cast<C*>(static_cast<Storable*>(backstore->find<C>(hl->get_path(), args...)));
        if (res)
            static_cast<Storable*>(res)->weight(current_weight);
        return res;
    }

    template <class C, typename ...Args> 
    C* ObjectBackStore::find(const Args& ...args) {
        BOOST_STATIC_ASSERT_MSG((boost::is_convertible<C*, Storable*>::value), 
            "You can only store objects that implement the Storable interface into the ObjectStore!");
        std::string name = str_cat(args...);
        shared<Storable>& res = (*_store)[name];
        if (res) 
            return static_cast<C*>(res.get());
        res.reset(new C());
        return static_cast<C*>(res.get());
    }

    template <class C> 
    std::vector<std::string> ObjectBackStore::list() const {
        std::vector<std::string> res;
        std::map<std::string, shared<Storable>>::const_iterator i;
        for (i = _store->begin(); i != _store->end(); i++) {
            if (dynamic_pointer_cast<C>(i->second)) 
                res.push_back(i->first);
        }
        return res;
    }

    template <class C> 
    shared<C> ObjectBackStore::get(std::string s) const {
        std::map<std::string, shared<Storable>>::const_iterator res;
        res = _store->find(s);
        if (res!=_store->end()) 
            return dynamic_pointer_cast<C>(res->second);
        return shared<C>();
    }


}
}

#endif
