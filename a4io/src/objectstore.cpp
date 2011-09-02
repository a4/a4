
#include "a4/objectstore.h"

#include <iostream>
#include <fstream>


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
}


hash_lookup::hash_lookup() {
    cc_key = NULL;
    ui_key = 0;
    for (int i = 0; i < size; i++) {
        data[i].value = NULL;
        data[i].cc_key = NULL;
        data[i].ui_key = 0;
        subdir[i] = NULL;
    };
}

void * & hash_lookup::lookup(const char * const index) {
    uintptr_t idx = reinterpret_cast<uintptr_t>(index) % size;
    uintptr_t idx0 = idx - 1;
    while (idx != idx0) {
        hash_lookup::hash_lookup_data & d = data[idx];
        if (d.cc_key == index && d.ui_key == 0) return d.value;
        if (d.value == NULL) { 
            d.ui_key = 0;
            d.cc_key = index; 
            if (is_writeable_pointer(index)) {
                std::cerr << "ERROR: Detected dynamically generated string in object lookup! "\
                             "Change it to a static string or use the slower get() lookup." << std::endl;
                assert(false);
            }
            return d.value; 
        };
        idx = (idx+1) % size;
    }
    std::cerr << "ERROR: Hash table of strings full - what are you doing???" << std::endl;
}

void * & hash_lookup::lookup(uint32_t index) {
    uintptr_t idx = index % size;
    uintptr_t idx0 = idx - 1;
    while (idx != idx0) {
        hash_lookup::hash_lookup_data & d = data[idx];
        if (d.ui_key == index && d.cc_key == NULL) return d.value;
        if (d.value == NULL) { d.ui_key = index; d.cc_key = NULL; return d.value; };
        idx = (idx+1) % size;
    }
    std::cerr << "ERROR: Hash table of strings full - what are you doing???" << std::endl;
}



