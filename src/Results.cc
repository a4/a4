#include "Results.h"

using namespace std;


Results::Results()
{
}

Results::~Results()
{
}

Results::H1Ptr Results::h1(std::string name) {
    return _h1[name];
};
        
Results::H1Ptr Results::h1(std::string name, const uint32_t &bins, const double &min, const double &max) {
    HMap::const_iterator search = _h1.find(name);
    if (search != _h1.end()) {
        return search->second;
    }
    H1Ptr h(new H1(bins, min, max));
    _h1[name] = h;
    return h;
};

void Results::add(const Results & source) {
    // first, merge all histos which are in this Result set
    HMap::const_iterator end = _h1.end();
    for (HMap::const_iterator it = _h1.begin(); it != end; ++it) {
        HMap::const_iterator search = source._h1.find(it->first);
        if (search != source._h1.end())
            it->second->add(*search->second);
    }
    // Add all histos which are only in the other Result set
    end = source._h1.end();
    for (HMap::const_iterator it = source._h1.begin(); it != end; ++it) {
        HMap::const_iterator search = _h1.find(it->first);
        if (search == source._h1.end())
            _h1[it->first] = it->second;
    }
}

void Results::print(std::ostream & out) const {
    HMap::const_iterator end = _h1.end();
    for (HMap::const_iterator it = _h1.begin(); it != end; ++it) {
        out << "Histogram '" << (it->first) << "':" << endl;
        it->second->print(out);
    }
}
