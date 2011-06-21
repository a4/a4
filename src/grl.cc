#include <iostream>
#include <fstream>

#include <a4/grl.h>

using namespace std;

GRL::GRL(string fn) {
    ifstream in(fn.c_str(), ios::in);
    uint32_t run, start, end;
    while(in >> run >> start >> end) {
        _data[run].insert(LBRange(end, start));
    }
};

bool GRL::pass(uint32_t run, uint32_t lb) {
    // returns the block where the "end lb" is at least as large as the current lb
    set<LBRange>::iterator block = _data[run].lower_bound(LBRange(lb, 0));
    if (block == _data[run].end()) return false;
    //cout << lb << " found " << (*block).second << " / " << (*block).first  << endl;
    // block.first is already larger/equal to lb, now check if block.second is smaller/equal:
    if ((*block).second <= lb) return true;
    return false;
}
