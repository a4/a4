#ifndef _A4_HASH_LOOKUP_IMPL_H_
#define _A4_HASH_LOOKUP_IMPL_H_

#include <a4/string.h>

template <typename... Args>
void * & hash_lookup::lookup(const char * const index, const Args& ...args) {
    uintptr_t idx = reinterpret_cast<uintptr_t>(index) % size;
    uintptr_t idx0 = (idx - 1) % size;
    while (idx != idx0) {
        hash_lookup * & sd = _subhash[idx];
        if (sd == NULL) {
            sd = new hash_lookup(str_cat(path, index));
            sd->cc_key = index;
            if (is_writeable_pointer(index)) {
                throw std::runtime_error("ERROR: Detected dynamically generated string in object lookup!\
                              Change it to a static string or use the slower get() lookup.");
            }
            return sd->lookup(args...); 
        };
        if (sd->cc_key == index && sd->ui_key == 0) return sd->lookup(args...);
        idx = (idx+1) % size;
    }
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.size.");
};

template <typename... Args>
void * & hash_lookup::lookup(uint32_t index, const Args& ...args) {
    uintptr_t idx = index % size;
    uintptr_t idx0 = (idx - 1) % size;
    while (idx != idx0) {
        hash_lookup * & sd = _subhash[idx];
        if (sd == NULL) { sd = new hash_lookup(str_cat(path, index)); sd->ui_key = index; return sd->lookup(args...); };
        if (sd->ui_key == index && sd->cc_key == NULL) return sd->lookup(args...);
        idx = (idx+1) % size;
    }
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.size.");
};

template <typename... Args>
hash_lookup * hash_lookup::subhash(const char * const index, const Args& ...args) {
    uintptr_t idx = reinterpret_cast<uintptr_t>(index) % size;
    uintptr_t idx0 = (idx - 1) % size;
    while (idx != idx0) {
        hash_lookup * & sd = _subhash[idx];
        if (sd == NULL) {
            sd = new hash_lookup(str_cat(path, index));
            sd->cc_key = index;
            if (is_writeable_pointer(index)) {
                throw std::runtime_error("ERROR: Detected dynamically generated string in object lookup!\
                              Change it to a static string or use the slower get() lookup.");
            }
            return sd->subhash(args...); 
        };
        if (sd->cc_key == index && sd->ui_key == 0) return sd->subhash(args...);
        idx = (idx+1) % size;
    }
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.size.");
};

template <typename... Args>
hash_lookup * hash_lookup::subhash(uint32_t index, const Args& ...args) {
    uintptr_t idx = index % size;
    uintptr_t idx0 = (idx - 1) % size;
    while (idx != idx0) {
        hash_lookup * & sd = _subhash[idx];
        if (sd == NULL) { sd = new hash_lookup(str_cat(path, index)); sd->ui_key = index; return sd->subhash(args...); };
        if (sd->ui_key == index && sd->cc_key == NULL) return sd->subhash(args...);
        idx = (idx+1) % size;
    }
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.size.");
};

#endif
