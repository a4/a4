#ifndef _A4_OBJECT_STORE_H_
#define _A4_OBJECT_STORE_H_

#include <boost/static_assert.hpp>

#include <a4/hash_lookup.h>
#include <a4/storable.h>

namespace a4{ namespace process{
    class ObjectBackStore;

    class ObjectStore {
        public:
            ObjectStore() {};
            template <class C, typename ...Args> C & T(const Args & ...args);
            template <class C, typename ...Args> C & find(const Args & ...args);
            template <class C, typename ...Args> C & slow(const Args & ...args);
            template <class C, typename ...Args> C & find_slow(const Args & ...args);
            template <typename ...Args, bool use=false> void T(const Args & ...args) { BOOST_STATIC_ASSERT_MSG(use, "Call T with a template parameter: S.T<H1>(''my histogram'')"); };
            template <typename ...Args, bool use=false> void find(const Args & ...args) { BOOST_STATIC_ASSERT_MSG(use, "Call find with a template parameter: S.find<H1>(''my histogram'')"); };
            template <typename ...Args> ObjectStore operator()(const Args & ...args);
        protected:
            ObjectStore(hash_lookup * hl, ObjectBackStore* bs) : hl(hl), backstore(bs) {};
            ObjectBackStore * backstore;
            hash_lookup * hl;
            friend class ObjectBackStore;
    };

    class ObjectBackStore {
        public:
            ObjectBackStore();
            ~ObjectBackStore();
            ObjectStore store();
            template <class C, typename ...Args> C * find(const Args & ...args);
            template <typename ...Args> ObjectStore operator()(const Args & ...args) { return store()(args...); };
            const shared<Storable> get(std::string s);
            template <class C> std::vector<std::string> list();
            /// Write all objects out into the given stream
            void to_stream(a4::io::A4OutputStream &outs) const;
            /// Read Storable Objects from input stream until new_metadata() is true
            void from_stream(a4::io::A4InputStream & ins);
        protected:
            hash_lookup hl;
            std::map<std::string, shared<Storable>> _store;
    };


};};

#include <a4/object_store_impl.h>

#endif
