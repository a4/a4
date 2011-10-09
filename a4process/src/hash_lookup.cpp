#include <iostream>
#include <fstream>

#include <a4/hash_lookup.h>

bool is_writeable_pointer(const char * _p) {
    uintptr_t p = reinterpret_cast<uintptr_t>(_p);
    std::ifstream map("/proc/self/maps");
    uintptr_t start, end; char minus, rflag, wflag;
    std::string line;
    while(getline(map,line)) {
        std::stringstream(line) >> std::hex >> start >> minus >> end >> rflag >> wflag;
        //std::cerr << std::hex << start << " to " << end << " flag " << wflag << std::endl;
        if (start < p && p < end) return (wflag == 'w');
    }
    std::cerr << "ptr at " << std::hex << p << " not in any mapped region!" << std::endl;
    return true;
};

hash_lookup::hash_lookup(std::string path) : path(path), cc_key(NULL), ui_key(0) {
    for (int i = 0; i < size; i++) _subhash[i] = NULL;
};

hash_lookup::~hash_lookup() {
    for (int i = 0; i < size; i++) if (_subhash[i]) delete _subhash[i];
};

hash_lookup * hash_lookup::subhash() { return this; };

void * & hash_lookup::lookup(const char * const index) {
    uintptr_t idx = reinterpret_cast<uintptr_t>(index) % size;
    uintptr_t idx0 = (idx - 1) % size;
    while (idx != idx0) {
        hash_lookup::hash_lookup_data & d = _data[idx];
        if (d.cc_key == index && d.ui_key == 0) return d.value;
        if (d.value == NULL) { 
            // Item not found, inserting
            d.ui_key = 0;
            d.cc_key = index; 
            if (is_writeable_pointer(index)) {
                throw std::runtime_error("ERROR: Detected dynamically generated string in object lookup! "\
                                     "Change it to a static string or use the slower get() lookup.");
            }
            return d.value; 
        };
        idx = (idx+1) % size;
    }
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.size.");
}

void * & hash_lookup::lookup(uint32_t index) {
    uintptr_t idx = index % size;
    uintptr_t idx0 = (idx - 1) % size;
    while (idx != idx0) {
        hash_lookup::hash_lookup_data & d = _data[idx];
        if (d.ui_key == index && d.cc_key == NULL) return d.value;
        if (d.value == NULL) {
            // Item not found, inserting
            d.ui_key = index; 
            d.cc_key = NULL; 
            return d.value;
        };
        idx = (idx+1) % size;
    }
    throw std::runtime_error("ERROR: Hash table full - check your code or increase hash_lookup.size.");
}


