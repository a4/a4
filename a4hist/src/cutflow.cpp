#include <iomanip>
#include <algorithm>
#include <map>

#include <a4/types.h>
#include <a4/cutflow.h>

using std::max;
using std::ios;
using std::setiosflags;
using std::setprecision;
using std::endl;
using std::map;
using std::vector;

namespace a4{ namespace hist{

Cutflow::Cutflow() {
    _fast_access.reset(new hash_lookup());
    _weights_squared.reset();
};

void Cutflow::constructor() {};

Cutflow::Cutflow(const Cutflow & h):
    _bin(h._bin),
    _cut_names(h._cut_names)
{
    _fast_access.reset(new hash_lookup());
    if (h._weights_squared) {
        _weights_squared.reset(new std::vector<double>);
        *_weights_squared = *h._weights_squared;
    }
}

Cutflow::~Cutflow() {}

Cutflow & Cutflow::__mul__(const double & w) {
    for(uint32_t bin = 0, bins = _bin.size(); bins > bin; ++bin)
        _bin[bin] *= w;
    if (!_weights_squared) {
        _weights_squared.reset(new vector<double>(_cut_names.size()));
        for(uint32_t i = 0; i < _cut_names.size(); i++) (*_weights_squared)[i] = _bin[i];
    }
    for(uint32_t bin = 0, bins = _cut_names.size(); bins > bin; ++bin)
        (*_weights_squared)[bin] *= w*w;
    return *this;
}


void Cutflow::fill_internal(const uintptr_t & id, const double & w) {
    double & current_value = _bin[id];
    if (_weights_squared)
        (*_weights_squared)[id] += w*w;
    else if (w != 1.0) {
        _weights_squared.reset(new vector<double>(_cut_names.size()));
        for(uint32_t i = 0; i < _cut_names.size(); i++) (*_weights_squared)[i] = _bin[i];
        (*_weights_squared)[id] += w*w;
    }
    current_value += w;
}

uintptr_t Cutflow::new_bin(std::string name) {
    uintptr_t idx = _cut_names.size();
    // if we land here, we have to save the name
    _cut_names.push_back(name);
    if (_weights_squared) (*_weights_squared).push_back(0);
    _bin.push_back(0);
    return idx + 1;
}

std::vector<Cutflow::CutNameCount> Cutflow::content() const {
    std::vector<Cutflow::CutNameCount> result;
    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        double w = _bin[i];
        if (_weights_squared) w = (*_weights_squared)[i];
        bool doublet = false;
        for(uint32_t j = 0; j < i; j++) {
            if (_cut_names[i] == _cut_names[j]) {
                result[j].count += _bin[i];
                result[j].weights_squared += w;
                doublet = true;
                break;
            }
        }
        if (!doublet) result.push_back(Cutflow::CutNameCount(_cut_names[i], _bin[i], w));
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
            out << std::setw(15) << setiosflags(ios::fixed) << setprecision(3) << _bin[i];
            out << " | ";
            out << _cut_names[i];
            out << endl;
        }
    }
}

Cutflow & Cutflow::__add__(const Cutflow & source) {
    map<std::string, int> cut_name_index;

    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        if (_cut_names[i].size() != 0) 
            cut_name_index[_cut_names[i]] = i+1; // +1 so 0 (default) is invalid
    }
    if (source._weights_squared && !_weights_squared) {
        _weights_squared.reset(new vector<double>(_cut_names.size()));
        for(uint32_t i = 0; i < _cut_names.size(); i++) (*_weights_squared)[i] = _bin[i];
    }

    // Add all cuts
    for(uint32_t i = 0; i < source._cut_names.size(); i++) {
        std::string name = source._cut_names[i];
        double count = source._bin[i];
        int index = cut_name_index[name] - 1;
        if (index == -1) {
            _cut_names.push_back(name);
            _bin.push_back(count);
            if (_weights_squared) {
                if (source._weights_squared) {
                    _weights_squared->push_back(source._weights_squared->operator[](i));
                } else {
                    _weights_squared->push_back(count);
                }
            }

        } else {
            _bin[index] += count;
        }
    }
    return *this;
}


void Cutflow::to_pb(bool clear_pb) {
    if (!clear_pb) pb.reset(new pb::Cutflow);
    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        if (_cut_names[i].size() != 0) {
            pb->add_counts_double(_bin[i]);
            pb->add_counts_double_names(_cut_names[i]);
            if (_weights_squared) {
                pb->add_weights_squared((*_weights_squared)[i]);
            }
        }
    }
    pb->set_title(title);
}

void Cutflow::from_pb() {
    foreach(double d, pb->counts_double()) _bin.push_back(d);
    foreach(std::string s, pb->counts_double_names()) _cut_names.push_back(s);    
    if (pb->weights_squared_size() > 0) {
        _weights_squared.reset(new vector<double>);
        foreach(double d, pb->weights_squared()) _weights_squared->push_back(d);
    }
    title = pb->title();
}

Cutflow & Cutflow::operator+=(const Cutflow &other) {
    return this->__add__(other);
}

};}; // namespace a4::hist;
