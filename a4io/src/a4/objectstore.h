#ifndef _A4_OBJECTSTORE_H_
#define _A4_OBJECTSTORE_H_

#include <vector>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <iostream>

class hash_lookup {
    public:
        hash_lookup();

        void * & lookup(const char * index);
        void * & lookup(uint32_t index);
        template <typename... Args>
        void * & lookup(const char * index, const Args& ...args);
        template <typename... Args>
        void * & lookup(uint32_t index, const Args& ...args);

    private:
        const static uintptr_t size = 1<<16;
        typedef struct { const char * cc_key; uint32_t ui_key; void * value; } hash_lookup_data;
        const char * cc_key;
        uint32_t ui_key;
        hash_lookup_data data[size];
        hash_lookup * subdir[size];
};

std::string str_printf(const char * s);
template<typename T, typename... Args, int N = sizeof...(Args)>
std::string str_printf(const char * s, const T& value, const Args&... args);

template<typename T, typename... Args, int N = sizeof...(Args)>
std::string str_cat(const T& s, const Args&... args);

template <typename STORE>
class ObjectStore {
    public:
        ObjectStore() {};

        // Retrieve a pointer to a T from the store with no checks, using its full name
        template <class T> 
        boost::shared_ptr<T> get(const std::string &name);

        // Map "name" to a pointer to T. Passes ownership of T to the Store.
        void set(const std::string &name, STORE* newly_created);

        void set(const std::string &name, boost::shared_ptr<STORE> obj);

        // Retrieve a pointer to T from the store with type checked
        template <class T> 
        boost::shared_ptr<T> get_checked(const std::string &name) const;

        // List all names in the Store
        std::vector<std::string> list() const;

        // List all names of a given class in the store
        template <class T>
        std::vector<std::string> list() const;

        // Fast access: Get name by format string and arguments
        template <class T, typename... Args>
        T & get_fmt(const char * name_fmt, const Args& ...args);

        // Fast access: Specify name by separate strings that will be concatenated
        template <class T, typename... Args>
        T & get_cat(const Args& ...args);

        // Fast access: Specify hint and name by separate strings that will be concatenated
        template <class T, typename... Args>
        T & get_hint(uint32_t hint, const Args& ...args);

    private:
        std::map<std::string, boost::shared_ptr<STORE> > main_storage;
        hash_lookup fast_lookup;
};

#include "a4/objectstore_impl.h"

#endif
