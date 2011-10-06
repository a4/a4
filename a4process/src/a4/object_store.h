#ifndef _A4_OBJECT_STORE_H_
#define _A4_OBJECT_STORE_H_

#include <boost/static_assert.hpp>

#include <a4/hash_lookup.h>
#include <a4/storable.h>

#define IS_FALSE(X) (sizeof(std::tuple<X...>) == -1)

namespace a4{ namespace process{
    class ObjectBackStore;

    /// Access Storable objects by name quickly.
    /// An ObjectStore is a fast-access interface to an ObjectBackStore.
    /// The keys to objects are always strings, but can be specified as
    /// separate arguments. The following calls are therefore equivalent:
    ///
    /// S.T<H1>("channel1/electron_1/pt")
    /// S.T<H1>("channel1/","electron_1/pt")
    /// S.T<H1>("channel", 1, "/electron_", 1, "/", pt")
    /// S("channel", 1).T<H1>("/electron_", 1, "/", pt")
    ///
    /// Use only string literals ("literal") and integers with S and (),
    /// dynamic strings are quite slow in construction. If you really
    /// have to use dynamic strings, use the _slow versions.
    ///
    /// The ObjectStore itself does not store any information, all the
    /// data is in the ObjectBackStore. Since it is only two pointers large,
    /// copying it is cheap and fast.
    class ObjectStore {
        public:
            ObjectStore() {};
            /// Get a reference to a C object identified by the concatenated args.
            /// If the object does not exist, it is created.
            /// Does not check the class type for performance reasons!
            template <class C, typename ...Args> C & T(const Args & ...args);
            /// Get a checked pointer to a C object identified by the concatenated args.
            /// If the object does not exist, it is created. If it exists but is
            /// of a non-dynamic-casteable type, find returns NULL.
            template <class C, typename ...Args> C * find(const Args & ...args);
            /// Version of T that works with dynamic strings (warning: very slow)
            template <class C, typename ...Args> C & T_slow(const Args & ...args);
            /// Version of find that works with dynamic strings (warning: very slow)
            template <class C, typename ...Args> C * find_slow(const Args & ...args);
            /// Gets the compiler to print a better warning if one forgets the template args
            template <typename ...Args> void T(const Args & ...args) { 
                BOOST_STATIC_ASSERT_MSG(IS_FALSE(Args), "ObjectStore::T must be called with a template parameter: S.T<H1>(\"my histogram\")"); 
            };
            /// Gets the compiler to print a better warning if one forgets the template args
            template <typename ...Args> void find(const Args & ...args) { 
                BOOST_STATIC_ASSERT_MSG(IS_FALSE(Args), "ObjectStore::find must be called with a template parameter: S.find<H1>(\"my histogram\")"); 
            };
            /// Get an ObjectStore with a "prefix" which can be handed to functions
            template <typename ...Args> ObjectStore operator()(const Args & ...args);
        protected:
            ObjectBackStore * backstore;
            hash_lookup * hl;
            /// Always create ObjectStores from ObjectBackStore.store() and ObjectStore("prefix/")
            ObjectStore(hash_lookup * hl, ObjectBackStore* bs) : hl(hl), backstore(bs) {};
            // Let ObjectBackStore use the protected constructor
            friend class ObjectBackStore;
    };

    /// A Map of strings to Storable objects.
    /// It provides very fast object access via the "ObjectStore" classes.
    /// In addition, it also provides methods to stream it to and from A4 Streams
    class ObjectBackStore {
        public:
            /// Create an empty ObjectBackStore
            ObjectBackStore();
            ~ObjectBackStore();
            /// Retrieve an ObjectStore "Reference" to this store.
            ObjectStore store();
            /// Directly retrieve an ObjectStore with the given prefix
            template <typename ...Args> ObjectStore operator()(const Args & ...args) {
                return store()(args...);
            };
            /// Get a shared pointer to the given object.
            template <class C> shared<C> get(std::string s) const;
            /// Get a list of all keys of all C objects in this store
            template <class C> std::vector<std::string> list() const;
            /// Get a list of all keys in this store
            std::vector<std::string> list() const;
            /// Write all objects out into the given stream
            void to_stream(a4::io::A4OutputStream &outs) const;
            /// Read Storable Objects from input stream until new_metadata() is true
            void from_stream(a4::io::A4InputStream & ins);
        protected:
            // The fast lookup table
            hash_lookup hl;
            // The actual store where the objects are kept
            std::map<std::string, shared<Storable>> _store;
            // Get a pointer to the given C object, creating it if it is not found.
            template <class C, typename ...Args> C * find(const Args & ...args);
            // Give ObjectStore access to find()
            friend class ObjectStore;
    };

};};

#include <a4/object_store_impl.h>

#endif
