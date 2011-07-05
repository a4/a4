#include "a4/results.h"

using namespace std;

Results::Results()
{
}

Results::~Results()
{
}


int Results::_fast_access_id_h1 = 0;
int Results::_fast_access_id_h2 = 0;
int Results::_fast_access_id_cf = 0;

H1Ptr Results::h1(string name) {
    return _h1[name];
};
        
H1Ptr Results::h1(string name, const uint32_t &bins, const double &min, const double &max) {
    H1Ptr & h = _h1[name];
    if (!h)
        h.reset(new H1(bins, min, max));
    return h;
};

H2Ptr Results::h2(string name) {
    return _h2[name];
};
        
H2Ptr Results::h2(string name, const uint32_t &xbins, const double &xmin, const double &xmax, const uint32_t &ybins, const double &ymin, const double &ymax) {
    H2Ptr & h = _h2[name];
    if (!h)
        h.reset(new H2(xbins, xmin, xmax, ybins, ymin, ymax));
    return h;
};

CutflowPtr Results::cf(string name) {
    CutflowPtr & h = _cf[name];
    if (!h)
        h.reset(new Cutflow());
    return h;
};

void Results::add(const Results & source) {
    {
        // first, merge all histos which are in this Result set
        map<string, H1Ptr>::const_iterator end = _h1.end();
        for (map<string, H1Ptr>::const_iterator it = _h1.begin(); it != end; ++it) {
            map<string, H1Ptr>::const_iterator other = source._h1.find(it->first);
            if (other != source._h1.end()) it->second->add(*other->second);
        }
        // Add all histos which are only in the other Result set
        // Copy them to make sure that we are allowed to add to them
        end = source._h1.end();
        for (map<string, H1Ptr>::const_iterator it = source._h1.begin(); it != end; ++it) {
            H1Ptr & h = _h1[it->first];
            if (!h) h.reset(new H1(*(it->second)));
        }
    }
    {
        // first, merge all histos which are in this Result set
        map<string, H2Ptr>::const_iterator end = _h2.end();
        for (map<string, H2Ptr>::const_iterator it = _h2.begin(); it != end; ++it) {
            map<string, H2Ptr>::const_iterator other = source._h2.find(it->first);
            if (other != source._h2.end()) it->second->add(*other->second);
        }
        // Add all histos which are only in the other Result set
        // Copy them to make sure that we are allowed to add to them
        end = source._h2.end();
        for (map<string, H2Ptr>::const_iterator it = source._h2.begin(); it != end; ++it) {
            H2Ptr & h = _h2[it->first];
            if (!h) h.reset(new H2(*(it->second)));
        }
    }
    {
        // first, merge all cfs which are in this Result set
        map<string, CutflowPtr>::const_iterator end = _cf.end();
        for (map<string, CutflowPtr>::const_iterator it = _cf.begin(); it != end; ++it) {
            map<string, CutflowPtr>::const_iterator other = source._cf.find(it->first);
            if (other != source._cf.end()) it->second->add(*other->second);
        }
        // Add all cfs which are only in the other Result set
        // Copy them to make sure that we are allowed to add to them
        end = source._cf.end();
        for (map<string, CutflowPtr>::const_iterator it = source._cf.begin(); it != end; ++it) {
            CutflowPtr & h = _cf[it->first];
            if (!h) h.reset(new Cutflow(*(it->second)));
        }
    }
}

void Results::print(std::ostream & out) const {
    if (false) {
        map<string, H1Ptr>::const_iterator end = _h1.end();
        for (map<string, H1Ptr>::const_iterator it = _h1.begin(); it != end; ++it) {
            out << "Histogram '" << (it->first) << "':" << endl;
            it->second->print(out);
        }
    }
    {
        map<string, CutflowPtr>::const_iterator end = _cf.end();
        for (map<string, CutflowPtr>::const_iterator it = _cf.begin(); it != end; ++it) {
            out << "Cutflow '" << (it->first) << "':" << endl;
            it->second->print(out);
        }
    }
}

std::vector<std::string> Results::h1_names() const {
    std::vector<std::string> result;
    map<string, H1Ptr>::const_iterator end = _h1.end();
    for (map<string, H1Ptr>::const_iterator it = _h1.begin(); it != end; ++it)
        result.push_back(it->first);
    return result;
}

std::vector<std::string> Results::h2_names() const {
    std::vector<std::string> result;
    map<string, H2Ptr>::const_iterator end = _h2.end();
    for (map<string, H2Ptr>::const_iterator it = _h2.begin(); it != end; ++it)
        result.push_back(it->first);
    return result;
}

std::vector<std::string> Results::cf_names() const {
    std::vector<std::string> result;
    map<string, CutflowPtr>::const_iterator end = _cf.end();
    for (map<string, CutflowPtr>::const_iterator it = _cf.begin(); it != end; ++it)
        result.push_back(it->first);
    return result;
}
