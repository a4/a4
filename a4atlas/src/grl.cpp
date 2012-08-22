#include <iostream>
#include <fstream>
#include <stdexcept>

#include <a4/grl.h>
#include <a4/types.h>

using namespace std;

namespace a4{ namespace atlas{

FileGRL::FileGRL(const string& fn) {
    ifstream in(fn.c_str(), ios::in);
    if (!in) {
        TERMINATE("Can not open '", fn, "'!");
    }
    uint32_t run, start, end;
    while(in >> run >> start >> end) {
        _data[run].insert(LBRange(end, start));
    }
    if (in.bad()) {
        TERMINATE("Reading from '", fn, "' failed! Is it really in (run, lb) format?");
    }
    if (_data.size() == 0) {
        TERMINATE("Cowardly refusing to use empty GRL from '", fn,
              "'. Is it really in (run, lb) format?");
    }
};

bool FileGRL::pass(const uint32_t& run, const uint32_t& lb) const {
    // First, check if the run is actually in the GRL:
    std::map<uint32_t, std::set<LBRange> >::const_iterator i_run = _data.find(run);
    if (i_run == _data.end())
        return false;
    const std::set<LBRange>& ranges = i_run->second;

    // Now, check if the lb is in one of the ranges
    // first get the range where the "end lb" is at least as large as the current lb
    set<LBRange>::const_iterator block = ranges.lower_bound(LBRange(lb, 0));
    if (block == ranges.end())
        return false; // the last end block happened before "lb"
    // block.first is already larger/equal to lb, now check if block.second is smaller/equal:
    if (block->second <= lb)
        return true;
    return false;
}

};}; // namespace atlas::a4;
