#include <iostream>
#include <fstream>

#include <boost/thread.hpp>

#include <a4/hash_lookup.h>

typedef boost::unique_lock<boost::mutex> Lock;

const uint32_t max_hashtable_size = 1 << 25; // 512 mb

// Pointers to regions of memory which are marked read only.
// We assume that these are compiled in and can therefore be cached.
static std::vector<uintptr_t> read_only_regions_start;
static std::vector<uintptr_t> read_only_regions_end;
static boost::mutex read_only_regions_mutex;

/// Read /prof/self/maps to determine the read only regions
/// Checks that the parameter `p` is present in one of those regions.
void refill_read_only_regions(uintptr_t p) {
    read_only_regions_start.clear();
    read_only_regions_end.clear();
    
    std::ifstream map("/proc/self/maps");
    uintptr_t start, end; 
    char minus, rflag, wflag;
    std::string line;
    bool found = false;
    
    while (getline(map,line)) {
        std::stringstream(line) >> std::hex >> start >> minus >> end >> rflag >> wflag;
        if (wflag != 'w') {
            read_only_regions_start.push_back(start);
            read_only_regions_end.push_back(end);
        }
        if (start < p && p < end) 
            found = true;
    }
    if (!found) {
        std::stringstream s;
        s << std::hex << p;
        FATAL("Pointer ", s.str(), " not in /proc/self/maps!");
    }
}

/// Check to see if the char pointer _p has the writable bit 
/// set in /proc/self/maps.
bool is_writeable_pointer(const char* _p) {
    Lock l(read_only_regions_mutex);
    uintptr_t p = reinterpret_cast<uintptr_t>(_p);
    
    // Try twice, refilling the regions the second time through
    // If it isn't found in the first time, it may haev been recently mapped.
    for (int c = 0; c < 2; c++) {        
        if (c == 1) 
            refill_read_only_regions(p);
            
        for(unsigned int i = 0; i < read_only_regions_start.size(); i++) {
            if (read_only_regions_start[i] < p && p < read_only_regions_end[i]) 
                return false;
        }
    }
    return true;
}

hash_lookup::hash_lookup(std::string path, int initial_size_log2) 
  :  _depth(0), _huid(0), _path(path), _files_size(1 << initial_size_log2), 
     _directories_size(1 << initial_size_log2) {
     
    _files = new hash_lookup_data[_files_size]();
    _directories = new hash_lookup_data[_directories_size]();
    _entries = _collisions = 0;
    _dir_entries = _dir_collisions = 0;
    _master = this;
    _huid_check = NULL;
    _check_huid = 0;
#ifdef HUID_CHECK
    _huid_check = new std::map<uint64_t, std::string>();
#endif
}

void hash_lookup::tear_down() {
    if(_depth != 0) return;
    
    for (unsigned int i = 0; i < _directories_size; i++)
        if (_directories[i].value) 
            delete static_cast<hash_lookup*>(_directories[i].value);
            
    delete[] _files;
    delete[] _directories;
    delete _huid_check;
}

/// Increase size of _files hashmap
bool hash_lookup::bump_up_files() {
    if (_depth != 0) return false;
    if (_entries < 0.1*_files_size) return false; // never occupy more than 10 times the memory
    if (_collisions < 0.05*_entries) return false; // only if more than 5% collisions
    //std::cerr << "BUMP: " << _files_size << "/" << _entries << "/" << _collisions << std::endl;

    hash_lookup_data* old_files = _files;
    int old_files_size = _files_size;
    
    _files_size *= 2;
    if (_files_size > max_hashtable_size)
        FATAL("Too many entries in hash table!");
    
    _collisions = 0;
    _files = new hash_lookup_data[_files_size]();
    
    for (int i = 0; i < old_files_size; i++) {
        hash_lookup_data & data = old_files[i];
        
        if (data.huid != 0 || data.value != NULL) {
            uintptr_t idx0 = idx_from_huid(data.huid, _files_size);
            uintptr_t idx = idx0;
            
            while (_files[idx].huid != 0 || _files[idx].value != NULL) 
                idx = (idx+1) & (_files_size-1);
                
            if (idx != idx0) 
                _collisions++;
                
            _files[idx] = data;
        }
    }
    
    delete[] old_files;
    
    for (unsigned int i = 0; i < _directories_size; i++)
        if (_directories[i].value)
            static_cast<hash_lookup*>(_directories[i].value)->update_from(this);
    
    //std::cerr << "POST BUMP: " << _files_size << "/" << _entries << "/" << _collisions << std::endl;
    return true;
}

/// Increase size of _directories hashmap
bool hash_lookup::bump_up_dirs() {
    if(_depth != 0) return false;
    
    if (_dir_entries < 0.1*_directories_size) return false; // never occupy more than 10 times the memory
    if (_dir_collisions < 0.05*_dir_entries) return false; // only if more than 5% collisions
    
    //std::cerr << "BUMP DIR: " << _directories_size << "/" << _dir_entries << "/" << _dir_collisions << std::endl;
    hash_lookup_data* old_directories = _directories;
    int old_dir_size = _directories_size;
    
    _directories_size *= 2;
    if (_directories_size > max_hashtable_size) 
        FATAL("Too many entries in hash table!");
    
    _directories = new hash_lookup_data[_directories_size]();
    
    for (int i = 0; i < old_dir_size; i++) {
        hash_lookup_data& data = old_directories[i];
        
        if (data.huid != 0 || data.value != NULL) {
            uintptr_t idx0 = idx_from_huid(data.huid, _directories_size);
            uintptr_t idx = idx0;
            
            while (_directories[idx].huid != 0 || _directories[idx].value != NULL) 
                idx = (idx+1) & (_directories_size-1);
                
            if (idx != idx0)
                _dir_collisions++;
                
            _directories[idx] = data;
            static_cast<hash_lookup*>(data.value)->update_from(this);
        }
    }
    
    delete[] old_directories;
    
    //std::cerr << "POST BUMP DIR: " << _directories_size << "/" << _dir_entries << "/" << _dir_collisions << std::endl;
    return true;
}

void hash_lookup::dump_stats() {
    int dcnt = 0, fcnt = 0;
    for (unsigned int i = 0; i < _directories_size; i++) 
        if (_directories[i].value) 
            dcnt++;
    for (unsigned int i = 0; i < _files_size; i++) 
        if (_files[i].huid || _files[i].value) 
            fcnt++;

    std::cout << "Depth/HUID: "<< _depth << "/" << _huid << std::endl;
    std::cout << "FILES:" << std::endl;
    std::cout << " * table size    = " << _files_size << std::endl;
    std::cout << " * nonzero cells = " << fcnt << std::endl;
    std::cout << " * entries       = " << _entries << std::endl;
    std::cout << " * collisions    = " << _collisions << std::endl;
    std::cout << "DIRECTORIES:" << std::endl;
    std::cout << " * table size = "<< _directories_size << std::endl;
    std::cout << " * nonzero cells = " << dcnt << std::endl;
    std::cout << " * entries       = " << _dir_entries << std::endl;
    std::cout << " * collisions    = " << _dir_collisions << std::endl;
};
