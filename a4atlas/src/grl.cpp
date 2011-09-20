#include <iostream>
#include <fstream>
#include <stdexcept>

#include <a4/grl.h>

using namespace std;

namespace a4{ namespace atlas{

GRL::GRL(const string & fn) {
    ifstream in(fn.c_str(), ios::in);
    if (!in) {
        cerr << "ERROR - a4::atlas::GRL::GRL - Can not open '" << fn << "'!" << endl;
        throw runtime_error("Can not open GRL file!");
    }
    uint32_t run, start, end;
    while(in >> run >> start >> end) {
        _data[run].insert(LBRange(end, start));
    }
    if (in.bad()) {
        cerr << "ERROR - a4::atlas::GRL::GRL - Reading data from '" << fn << "' failed! Is it really in (run, lb) format?" << endl;
        throw runtime_error("Can not read GRL file!");
    }
    if (_data.size() == 0) {
        cerr << "ERROR - a4::atlas::GRL::GRL - Cowardly refusing to use empty GRL from '" << fn << "'. Is it really in (run, lb) format?" << endl;
        throw runtime_error("Empty GRL file!");
    }
};

bool GRL::pass(const uint32_t &run, const uint32_t &lb) const {
    // First, check if the run is actually in the GRL:
    std::map<uint32_t, std::set<LBRange> >::const_iterator i_run = _data.find(run);
    if (i_run == _data.end()) return false;
    const std::set<LBRange> & ranges = i_run->second;

    // Now, check if the lb is in one of the ranges
    // first get the range where the "end lb" is at least as large as the current lb
    set<LBRange>::const_iterator block = ranges.lower_bound(LBRange(lb, 0));
    if (block == ranges.end()) return false; // the last end block happened before "lb"
    // block.first is already larger/equal to lb, now check if block.second is smaller/equal:
    if (block->second <= lb) return true;
    return false;
}

};}; // namespace atlas::a4;
