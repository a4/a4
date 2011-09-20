#include <a4/cutflow.h>

#include <google/protobuf/descriptor.h>

#include <a4/proto/io/Cutflow.pb.h>

#include <boost/foreach.hpp>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <inttypes.h>

using std::max;
using std::ios;
using std::setiosflags;
using std::setprecision;
using std::endl;
using std::map;
using std::vector;

int Cutflow::_fast_access_id = 0;

Cutflow::Cutflow() {};

Cutflow::Cutflow(const Cutflow & h):
    _fast_access_bin(h._fast_access_bin),
    _cut_names(h._cut_names)
{
    if (h._weights_squared) {
        _weights_squared.reset(new std::vector<double>);
        *_weights_squared = *h._weights_squared;
    }
}

Cutflow::~Cutflow()
{
}

ResultType & Cutflow::__mul__(const double & w) {
    for(uint32_t bin = 0, bins = _fast_access_bin.size(); bins > bin; ++bin)
        _fast_access_bin[bin] *= w;
    if (!_weights_squared) {
        _weights_squared.reset(new vector<double>(_cut_names.size()));
        for(uint32_t i = 0; i < _cut_names.size(); i++) (*_weights_squared)[i] = _fast_access_bin[i];
    }
    for(uint32_t bin = 0, bins = _cut_names.size(); bins > bin; ++bin)
        (*_weights_squared)[bin] *= w*w;
    return *this;
}

void Cutflow::fill(const int & id, const std::string & name, const double w) {
    try {
        double & current_value = _fast_access_bin.at(id);
        if (current_value != 0) {
            if (_weights_squared)
                (*_weights_squared)[id] += w*w;
            else if (w != 1.0) {
                _weights_squared.reset(new vector<double>(_cut_names.size()));
                for(uint32_t i = 0; i < _cut_names.size(); i++) (*_weights_squared)[i] = _fast_access_bin[i];
                (*_weights_squared)[id] += w*w;
            }
            current_value += w;
            return;
        }
    } catch (std::out_of_range & oor) {
        _fast_access_bin.resize(id+1);
        if (_weights_squared) _weights_squared->resize(id+1);
        _cut_names.resize(id+1);
    }
    // if we land here, we have to save the name
    _cut_names[id] = name;
    if (_weights_squared)
        (*_weights_squared)[id] += w*w;
    else if (w != 1.0) {
        _weights_squared.reset(new vector<double>(_cut_names.size()));
        for(uint32_t i = 0; i < _cut_names.size(); i++) (*_weights_squared)[i] = _fast_access_bin[i];
        (*_weights_squared)[id] += w*w;
    }
    _fast_access_bin[id] += w;
}

std::vector<Cutflow::CutNameCount> Cutflow::content() const {
    std::vector<Cutflow::CutNameCount> result;
    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        double w = _fast_access_bin[i];
        if (_weights_squared) w = (*_weights_squared)[i];
        bool doublet = false;
        for(uint32_t j = 0; j < i; j++) {
            if (_cut_names[i] == _cut_names[j]) {
                result[j].count += _fast_access_bin[i];
                result[j].weights_squared += w;
                doublet = true;
                break;
            }
        }
        if (!doublet) result.push_back(Cutflow::CutNameCount(_cut_names[i], _fast_access_bin[i], w));
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

ResultType & Cutflow::__add__(const ResultType & _source) {
    const Cutflow & source = dynamic_cast<const Cutflow &>(_source);
    map<std::string, int> cut_name_index;

    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        if (_cut_names[i].size() != 0) 
            cut_name_index[_cut_names[i]] = i+1; // +1 so 0 (default) is invalid
    }
    if (source._weights_squared && !_weights_squared) {
        _weights_squared.reset(new vector<double>(_cut_names.size()));
        for(uint32_t i = 0; i < _cut_names.size(); i++) (*_weights_squared)[i] = _fast_access_bin[i];
    }

    // Add all cuts
    for(uint32_t i = 0; i < source._cut_names.size(); i++) {
        std::string name = source._cut_names[i];
        double count = source._fast_access_bin[i];
        int index = cut_name_index[name] - 1;
        if (index == -1) {
            _cut_names.push_back(name);
            _fast_access_bin.push_back(count);
            if (_weights_squared) {
                if (source._weights_squared) {
                    _weights_squared->push_back(source._weights_squared->operator[](i));
                } else {
                    _weights_squared->push_back(count);
                }
            }

        } else {
            _fast_access_bin[index] += count;
        }
    }
    return *this;
}


MessagePtr Cutflow::get_message() {
    boost::shared_ptr<a4::io::Cutflow> cf(new a4::io::Cutflow);
    for(uint32_t i = 0; i < _cut_names.size(); i++) {
        if (_cut_names[i].size() != 0) {
            cf->add_counts_double(_fast_access_bin[i]);
            cf->add_counts_double_names(_cut_names[i]);
            if (_weights_squared) {
                cf->add_weights_squared((*_weights_squared)[i]);
            }
        }
    }
    return cf;
}

Cutflow::Cutflow(Message& m) {
    a4::io::Cutflow * msg = dynamic_cast<a4::io::Cutflow*>(&m);
    BOOST_FOREACH(double d, msg->counts_double()) _fast_access_bin.push_back(d);
    BOOST_FOREACH(std::string s, msg->counts_double_names()) _cut_names.push_back(s);    
    if (msg->weights_squared_size() > 0) {
        _weights_squared.reset(new vector<double>);
        BOOST_FOREACH(double d, msg->weights_squared()) _weights_squared->push_back(d);
    }
}

