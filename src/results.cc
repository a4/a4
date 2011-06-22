#include "a4/results.h"

using namespace std;


Results::Results()
{
    _h1.reset(new HMap());
}

Results::~Results()
{
}

H1Ptr Results::h1(string name) {
    return (*_h1)[name];
};
        
H1Ptr Results::h1(string name, const uint32_t &bins, const double &min, const double &max) {
    H1Ptr & h = (*_h1)[name];
    if (!h)
        h.reset(new H1(bins, min, max));
    return h;
};

void Results::add(const Results & source) {
    // first, merge all histos which are in this Result set
    HMap::const_iterator end = _h1->end();
    for (HMap::const_iterator it = _h1->begin(); it != end; ++it) {
        H1Ptr & h = (*source._h1)[it->first];
        if (h) it->second->add(*h);
    }
    // Add all histos which are only in the other Result set
    // Copy them to make sure that we are allowed to add to them
    end = source._h1->end();
    for (HMap::const_iterator it = source._h1->begin(); it != end; ++it) {
        H1Ptr & h = (*_h1)[it->first];
        if (!h) h.reset(new H1(*(it->second)));
    }
}

void Results::print(std::ostream & out) const {
    HMap::const_iterator end = _h1->end();
    for (HMap::const_iterator it = _h1->begin(); it != end; ++it) {
        out << "Histogram '" << (it->first) << "':" << endl;
        it->second->print(out);
    }
}

std::vector<std::string> Results::names() const {
    std::vector<std::string> result;
    HMap::const_iterator end = _h1->end();
    for (HMap::const_iterator it = _h1->begin(); it != end; ++it)
        result.push_back(it->first);
    return result;
}
