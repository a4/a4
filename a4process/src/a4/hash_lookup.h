#ifndef _A4_HASH_LOOKUP_H_
#define _A4_HASH_LOOKUP_H_

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

/// \internal
/// hash_lookup accelerates lookup of concatenated strings and numbers
/// as occur often in analysis. It consists of a sequence of hash tables
/// where each string and number is looked up.
/// The implemtation uses "variadic templates", a very useful C++11 feature
/// On insertion, it asserts that the const char * used is in a read-only
/// section of memory to make sure no accidental overwriting takes place.
///
/// This class currently heavily trades memory against speed, so there may be
/// as much as 64k consumed per entry (usually less).
/// There are ways to reduce this dramatically, but this is not a priority right now.
class hash_lookup {
    public:
        /// Create an empty hash table with the @path prefix.
        hash_lookup(std::string path="");

        /// Lookup a piece of _data_ by const char * index
        void * & lookup(const char * index);
        /// Lookup a piece of _data_ by uint32_t index
        void * & lookup(uint32_t index);
        /// Lookup a _subhash_ by const char * index
        template <typename... Args>
        void * & lookup(const char * index, const Args& ...args);
        /// Lookup a _subhash_ uint32_t index
        template <typename... Args>
        void * & lookup(uint32_t index, const Args& ...args);

        /// Lookup a _subhash_ by const char * index
        template <typename... Args>
        hash_lookup * subhash(const char * index, const Args& ...args);
        /// Lookup a _subhash_ uint32_t index
        template <typename... Args>
        hash_lookup * subhash(uint32_t index, const Args& ...args);
        hash_lookup * subhash();

        const std::string & get_path() const {return path;};

    private:
        void zero();
        std::string path;
        const static uintptr_t size = 1<<16;
        typedef struct { const char * cc_key; uint32_t ui_key; void * value; } hash_lookup_data;
        const char * cc_key;
        uint32_t ui_key;
        hash_lookup_data _data[size];
        hash_lookup * _subhash[size];
};

bool is_writeable_pointer(const char * _p);

template<typename T, typename... Args>
std::string str_printf(const char * s, const T& value, const Args&... args);

template<typename T, typename... Args>
std::string str_cat(const T& s, const Args&... args);

#include <a4/hash_lookup_impl.h>

#endif
