#ifndef _A4_OBJECT_STORE_IMPL_H_
#define _A4_OBJECT_STORE_IMPL_H_

#include <boost/type_traits.hpp>

namespace a4{ namespace process{
    using a4::process::Storable;

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

    template <class C, typename ...Args> C & ObjectStore::find(const Args & ...args) {
        Storable * b = static_cast<Storable*>(&T(args...));
        return *dynamic_cast<C*>(b);
    }

    template <class C, typename ...Args> C & ObjectStore::find_slow(const Args & ...args) {
        Storable * b = static_cast<Storable*>(backstore->find<C>(hl->get_path(), args...));
        return *dynamic_cast<C*>(b);
    };

    template <class C, typename ...Args> C * ObjectBackStore::find(const Args & ...args) {
        //BOOST_STATIC_ASSERT_MSG((boost::is_convertible<C*, Storable*>::value), "You can only store objects that implement the Storable interface into the ObjectStore!");
        std::string name = str_cat(args...);
        void * & res = _store[name];
        if (res) return static_cast<C*>(res);
        C* p = new C();
        res = p;
        return p;
    };
};};

#endif
