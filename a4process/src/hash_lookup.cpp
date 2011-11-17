#include <iostream>
#include <fstream>

#include <a4/hash_lookup.h>

static std::vector<uintptr_t> read_only_regions_start;
static std::vector<uintptr_t> read_only_regions_end;

void refill_read_only_regions(uintptr_t p) {
    read_only_regions_start.clear();
    read_only_regions_end.clear();
    std::ifstream map("/proc/self/maps");
    uintptr_t start, end; char minus, rflag, wflag;
    std::string line;
    bool found = false;
    while(getline(map,line)) {
        std::stringstream(line) >> std::hex >> start >> minus >> end >> rflag >> wflag;
        //std::cerr << std::hex << start << " to " << end << " flag " << wflag << std::endl;
        if (wflag != 'w') {
            read_only_regions_start.push_back(start);
            read_only_regions_end.push_back(end);
        }
        if (start < p && p < end) found = true;
    }
    if (!found) std::cerr << "ptr at " << std::hex << p << " not in any mapped region!" << std::endl;
}

bool is_writeable_pointer(const char * _p) {
    uintptr_t p = reinterpret_cast<uintptr_t>(_p);
    for (int c = 0; c < 2; c++) { // try twice, refilling the regions the second time through
        if (c == 1) refill_read_only_regions(p);
        for(int i = 0; i < read_only_regions_start.size(); i++) {
            if (read_only_regions_start[i] < p && p < read_only_regions_end[i]) return false;
        }
    }
    return true;
};

hash_lookup::hash_lookup(std::string path) : _depth(0), _huid(0), _path(path) {
    _files = new hash_lookup_data[files_size]();
    _directories = new hash_lookup_data[directories_size]();
    _collisions = 0;
};

void hash_lookup::tear_down() {
    if(_depth != 0) return;
    for (int i = 0; i < directories_size; i++) {
        if (_directories[i].value) delete static_cast<hash_lookup*>(_directories[i].value);
    }
    delete[] _files;
    delete[] _directories;
};

void hash_lookup::dump_stats() {
    std::cout << "My depth: "<< _depth << std::endl;
    std::cout << "My huid: "<< std::hex << _huid << std::dec << std::endl;
    std::cout << "Size of directories table: "<< directories_size << std::endl;
    std::cout << "Size of files table: "<< files_size << std::endl;
    int dcnt = 0, fcnt = 0;
    for (int i = 0; i < directories_size; i++) if (_directories[i].value) dcnt++;
    for (int i = 0; i < files_size; i++) if (_files[i].huid || _files[i].value) fcnt++;
    std::cout << "# dir "<< dcnt << std::endl;
    std::cout << "# fls "<< fcnt << std::endl;
    std::cout << "# collisions "<< _collisions << std::endl;
};
