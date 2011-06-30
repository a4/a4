#include <a4/cutflow.h>

#include <boost/foreach.hpp>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <inttypes.h>

using std::max;
using std::ios;
using std::setiosflags;
using std::setprecision;
using std::endl;

int Cutflow::_fast_access_id = 0;

Cutflow::Cutflow() {};

Cutflow::Cutflow(const Cutflow & h):
    _fast_access_bin(h._fast_access_bin),
    _cut_names(h._cut_names)
{
}

Cutflow::~Cutflow()
{
}

void Cutflow::fill(const int & id, const std::string & name, const double w) {
    try {
        double & current_value = _fast_access_bin.at(id);
        if (current_value != 0) {
            current_value += w;
            return;                    
        }
    } catch (std::out_of_range & oor) {
        _fast_access_bin.resize(id+1);
        _cut_names.resize(id+1);
    }
    // if we land here, we have to save the name
    _cut_names[id] = name;
    _fast_access_bin[id] += w;
}

std::vector<Cutflow::CutNameCount> Cutflow::content() const {
    std::vector<Cutflow::CutNameCount> result;
    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        result.push_back(Cutflow::CutNameCount(_cut_names[i], _fast_access_bin[i]));
    }
    return result;
}

void Cutflow::print(std::ostream &out) const
{
    size_t max_namelen = 1;
    for (uint32_t i = 0; i < _cut_names.size(); i++)
        max_namelen = max(max_namelen, _cut_names[i].size());

    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        if (_cut_names[i].size() != 0) {
            out << std::setw(15) << setiosflags(ios::fixed) << setprecision(3) << _fast_access_bin[i];
            out << " | ";
            out << _cut_names[i];
            out << endl;
        }
    }
}

void Cutflow::add(const Cutflow &source)
{
    if (source._cut_names.size() > _cut_names.size()) {
        _fast_access_bin.resize(source._cut_names.size());
        _cut_names.resize(source._cut_names.size());
    }
    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        if (source._cut_names[i].size() == 0) {
            // do nothing
        } else if (_cut_names[i].size() == 0) {
            _cut_names[i] = source._cut_names[i];
            _fast_access_bin[i] = source._fast_access_bin[i];
        } else if (_cut_names[i] != source._cut_names[i]) {
            throw std::runtime_error("Trying to add Cutflows with unequal cut names!");
        } else {
            _fast_access_bin[i] += source._fast_access_bin[i];
        }
    }
}
