#ifndef _A4_HASH_LOOKUP_IMPL_H_
#define _A4_HASH_LOOKUP_IMPL_H_

#include <a4/debug.h>
#include <a4/string.h>
#include <iostream>

// Get a hopefully unique ID 
template <typename T, typename... Args>
uint64_t get_huid(const T& c, const Args& ...args);

const auto hashmap_magic_number = 0x9e3779b97f4a7c13LL;

// Get a hopefully unique ID 
// This function is critical!
inline static uint64_t get_huid() { return 0; }
template <typename... Args>
uint64_t get_huid(const char* c, const Args& ...args) {
    uintptr_t v = reinterpret_cast<uintptr_t>(c);
    return (get_huid(args...) << 1) ^ (v*hashmap_magic_number);
}
template <typename T, typename... Args>
uint64_t get_huid(const T& c, const Args& ...args) {
    uint64_t v = c;
    return (get_huid(args...) << 1) ^ (v*hashmap_magic_number);
}

// Checked str_cat
inline static void _checked_stream_in(std::stringstream& ss) {}

template<typename T, typename... Args>
void _checked_stream_in(std::stringstream& ss, const T& s, const Args&... args);

template<typename... Args>
void _checked_stream_in(std::stringstream& ss, const char* s, const Args&... args) {
    if (is_writeable_pointer(s)) {
        FATAL("Detected dynamically generated string in object lookup! "
              "Change it to a static string or use the slower get() lookup.");
    }
    ss << s;
    _checked_stream_in(ss, args...);
}

template<typename... Args>
void _checked_stream_in(std::stringstream& ss, std::string& s, const Args&... args) {
    FATAL("Detected std::string class in object lookup! "
          "Change it to a const char* or use the slower get() lookup.");
}

template<typename T, typename... Args>
void _checked_stream_in(std::stringstream& ss, const T& s, const Args&... args) {
    ss << s;
    _checked_stream_in(ss, args...);
}

template<typename... Args>
std::string checked_str_cat(const Args&... args) {
    std::stringstream ss;
    _checked_stream_in(ss, args...);
    return ss.str();
}

inline uintptr_t hash_lookup::idx_from_huid(uint64_t huid, int size) {
    return (size-1) & ((huid >> 48) ^ (huid >> 32) ^ (huid >> 16));
}

template <typename... Args>
void*& hash_lookup::lookup(const Args& ...args) {
    uint64_t huid = _huid ^ (get_huid(args...) << _depth);
    uintptr_t idx = idx_from_huid(huid, _files_size);
    uintptr_t idx0 = idx;
    
    do {
        hash_lookup_data& data = _files[idx];
        
        if (data.huid == huid) {
#ifdef HUID_CHECK
#ifdef HUID_CHECK_ONLY_ONCE
            if (_check_huid > 0) 
#endif
            {
                std::string full_name = _path + str_cat(args...);
                std::string& other = (*_master->_huid_check)[huid];
                if (other == "") {
                    other = full_name;
                } else if (other != full_name) {
                    FATAL("Hopefully Unique ID is not Unique: ", full_name, 
                          " has same HUID than ", other, ". Please report this "
                          "to the A4 developers!");
                }
                _check_huid--;
            }
#endif
            return data.value;
        } else if (data.huid == 0 && data.value == NULL) {
            std::string full_name = _path + checked_str_cat(args...); // just check that no dynamic stuff was used
#ifdef HUID_CHECK
            _check_huid++;
            (*_master->_huid_check)[huid] = full_name;
#endif
            if (idx != idx0) {
                _master->_collisions++;
                if(_master->bump_up_files()) 
                    return lookup(args...);
            }
            
            data.huid = huid;
            _master->_entries++;
            return data.value;
        }
        
        idx = (idx+1) & (_files_size-1);
    } while (idx != idx0);
    
    FATAL("ERROR: Hash table full - check your code or "
          "increase hash_lookup.files_size.");
};

template <typename... Args>
hash_lookup* hash_lookup::subhash(const Args& ...args) {
    uint64_t huid = _huid ^ (get_huid(args...) << _depth);
    uintptr_t idx = idx_from_huid(huid, _directories_size);
    uintptr_t idx0 = idx;
    
    do {
        hash_lookup_data& data = _directories[idx];
        
        if (data.huid == huid) {
#ifdef HUID_CHECK
#ifdef HUID_CHECK_ONLY_ONCE
            if (_check_huid > 0) 
#endif
            {
                std::string full_name = _path + str_cat(args...);
                std::string& other = (*_master->_huid_check)[huid];
                if (other == "") {
                    other = full_name;
                } else if (other != full_name) {
                    FATAL("Hopefully Unique ID is not Unique: ", full_name, 
                          " has same HUID than ", other, ". Please report this "
                          "to the A4 developers!");
                }
                _check_huid--;
            }
#endif
            return static_cast<hash_lookup*>(data.value);
        }
        
        if (data.huid == 0 && data.value == NULL) {
            std::string new_path = _path + checked_str_cat(args...);
#ifdef HUID_CHECK
            _check_huid++;
            (*_master->_huid_check)[huid] = new_path;
#endif
            if (idx != idx0) {
                _master->_dir_collisions++;
                if (_master->bump_up_dirs()) 
                    return subhash(args...);
            }
            data.huid = huid;
            _master->_dir_entries++;
            data.value = new hash_lookup(huid, new_path, this);
            return static_cast<hash_lookup*>(data.value);
        }
        
        idx = (idx+1)& (_directories_size-1);
    } while (idx != idx0);
    
    FATAL("Hash table full - check your code or "
          "increase hash_lookup.directories_size.");
}

#endif
