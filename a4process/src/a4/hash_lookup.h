#ifndef _A4_HASH_LOOKUP_H_
#define _A4_HASH_LOOKUP_H_

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

#define HUID_CHECK
#define HUID_CHECK_ONLY_ONCE

/// \internal
/// hash_lookup accelerates lookup of concatenated strings and numbers
/// as occur often in analysis. It consists of a sequence of hash tables
/// where each string and number is looked up.
/// The implemtation uses "variadic templates", a very useful C++11 feature
/// On insertion, it asserts that the const char* used is in a read-only
/// section of memory to make sure no accidental overwriting takes place.
///
/// This class currently heavily trades memory against speed, so there may be
/// as much as 64k consumed per entry (usually less).
/// There are ways to reduce this dramatically, but this is not a priority right now.
class hash_lookup {
    public:
        /// Create an empty hash table with the @path prefix.
        hash_lookup(std::string path="", int initial_size_log2=16);
        ~hash_lookup() { if (_depth == 0) tear_down(); };

        /// Lookup a file (leaf)
        template <typename... Args>
        void *& lookup(const Args& ...args);

        /// Lookup a directory
        template <typename... Args>
        hash_lookup* subhash(const Args& ...args);

        const std::string& get_path() const { return _path; }

        void dump_stats();
    private:
        typedef struct hash_lookup_data_s {
            hash_lookup_data_s() : huid(0), value(NULL) {};
            uint64_t huid; // hopefully unique ID
            void* value;  // stored value
        } hash_lookup_data;

        hash_lookup(uint64_t huid, std::string path, hash_lookup* parent) :
            _depth(parent->_depth + 1),
            _huid(huid),
            _path(path)
        {
            update_from(parent);
            _check_huid = 0;
        };

        void update_from(hash_lookup* master) {
            _files_size = master->_files_size;
            _directories_size = master->_directories_size;
            _files = master->_files;
            _directories = master->_directories;
            _master = master->_master;
        };

        void tear_down();
        hash_lookup* subhash() { return this; }
        uintptr_t idx_from_huid(uint64_t huid, int size);

        bool bump_up_files();
        bool bump_up_dirs();

        int _depth;
        uint64_t _huid;
        std::string _path;
        uintptr_t _files_size, _directories_size;
        hash_lookup_data* _files;
        hash_lookup_data* _directories;
        hash_lookup* _master;
        int _entries, _collisions;
        int _dir_entries, _dir_collisions;
        std::map<uint64_t, std::string>* _huid_check;
        int _check_huid;
};

bool is_writeable_pointer(const char* _p);

#include <a4/hash_lookup_impl.h>

#endif
