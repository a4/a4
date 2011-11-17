#ifndef _A4_HASH_LOOKUP_IMPL_H_
#define _A4_HASH_LOOKUP_IMPL_H_

#include <a4/string.h>
#include <iostream>

// Get a hopefully unique ID 
template <typename T, typename... Args>
uint64_t get_huid(const T& c, const Args& ...args);

// Get a hopefully unique ID 
// This function is critical!
inline static uint64_t get_huid() { return 0; }
template <typename... Args>
uint64_t get_huid(const char * c, const Args& ...args) {
    uintptr_t v = reinterpret_cast<uintptr_t>(c);
    return (get_huid(args...) << 1) ^ (v*0x9e3779b97f4a7c13LL);
}
template <typename T, typename... Args>
uint64_t get_huid(const T& c, const Args& ...args) {
    uint64_t v = c;
    return (get_huid(args...) << 1) ^ (v*0x9e3779b97f4a7c13LL);
}

// Checked str_cat
inline static void _checked_stream_in(std::stringstream & ss) {};

template<typename T, typename... Args>
void _checked_stream_in(std::stringstream & ss, const T & s, const Args&... args);

template<typename... Args>
void _checked_stream_in(std::stringstream & ss, const char * s, const Args&... args) {
    if (is_writeable_pointer(s)) {
        throw std::runtime_error("ERROR: Detected dynamically generated string in object lookup!\
                                  Change it to a static string or use the slower get() lookup.");
    }
    ss << s;
    _checked_stream_in(ss, args...);
}

template<typename... Args>
void _checked_stream_in(std::stringstream & ss, std::string & s, const Args&... args) {
    throw std::runtime_error("ERROR: Detected std::string class in object lookup!\
                              Change it to a const char * or use the slower get() lookup.");
}

template<typename T, typename... Args>
void _checked_stream_in(std::stringstream & ss, const T & s, const Args&... args) {
    ss << s;
    _checked_stream_in(ss, args...);
}



template<typename... Args>
std::string checked_str_cat(const Args&... args) {
    std::stringstream ss;
    _checked_stream_in(ss, args...);
    return ss.str();
};

template <typename... Args>
void * & hash_lookup::lookup(const Args& ...args) {
    int64_t huid = _huid ^ (get_huid(args...) << _depth);
    //uintptr_t idx = huid & (files_size-1);
    uintptr_t idx = (files_size-1) & ((huid >> 48) ^ (huid >> 32) ^ (huid >> 16));
    uintptr_t idx0 = idx;
    do {
        hash_lookup_data & data = _files[idx];
        if (data.huid == huid) {
            return data.value;
        } else if (data.huid == 0 && data.value == NULL) {
            checked_str_cat(args...); // just check that no dynamic stuff was used
            data.huid = huid;
            if (idx != idx0) _collisions++;
            return data.value;
        }
        idx = (idx+1) & (files_size-1);
    } while (idx != idx0);
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.files_size.");
};

template <typename... Args>
hash_lookup * hash_lookup::subhash(const Args& ...args) {
    int64_t huid = _huid ^ get_huid(args...) << (_depth*2);
    uintptr_t idx = huid % directories_size;
    uintptr_t idx0 = (huid-1) % directories_size;
    while (idx != idx0) {
        hash_lookup_data & data = _directories[idx];
        if (data.huid == huid) return static_cast<hash_lookup*>(data.value);
        if (data.huid == 0 && data.value == NULL) {
            data.huid = huid;
            data.value = new hash_lookup(_depth+1, huid, _path + checked_str_cat(args...), _files, _directories);
            return static_cast<hash_lookup*>(data.value);
        }
        idx = (idx+1) % directories_size;
    }
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.directories_size.");
};

#endif
