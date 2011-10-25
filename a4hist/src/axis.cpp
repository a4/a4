#include <algorithm>
#include <iostream>
#include <limits>

#include "a4/axis.h"
#include "a4/hist/Histograms.pb.h"

using namespace std;
namespace a4{ namespace hist{

unique<Axis> Axis::from_proto(const pb::Axis & msg) {
    if (msg.has_bins())
        return unique<Axis>(new SimpleAxis(msg));
    else
        return unique<Axis>(new VariableAxis(msg));
}

SimpleAxis::SimpleAxis() {};

SimpleAxis::SimpleAxis(const uint32_t &bins, const double &min, const double &max):
    _min(min),
    _max(max),
    _bins(bins),
    _delta(bins == 0 ? 0 : (max - min)/bins)
{   
};

SimpleAxis::SimpleAxis(const SimpleAxis & a):

    _min(a._min),
    _max(a._max),
    _bins(a._bins),
    _delta(a._delta)
{
    label = a.label;
}

SimpleAxis::~SimpleAxis() 
{
};

SimpleAxis::SimpleAxis(const pb::Axis & msg) {
    label = msg.label();
    _min = msg.min();
    _max = msg.max();
    _bins = msg.bins();
    _delta = _bins == 0 ? 0 : (_max - _min)/_bins;
};

unique<pb::Axis> SimpleAxis::get_proto() {
    unique<pb::Axis> axis(new pb::Axis);
    axis->set_bins(_bins);
    axis->set_min(_min);
    axis->set_max(_max);
    axis->set_label(label);
    return axis;
};

bool SimpleAxis::sane() const
{
    return (_bins > 0 && _min <= _max);
}

uint32_t SimpleAxis::find_bin(const double &x) const
{
    if (!sane())
        return 0;

    if (x < _min)
        return 0;

    if (x > _max)
        return 1 + _bins;

    return 1 + ((x - _min) / _delta);
}

// Helpers
//
std::ostream &operator<<(std::ostream &out, const Axis &axis)
{
    return out << axis.label << " - " << axis.bins() << " bins in range ["
        << axis.min() << "," << axis.max() << "]";
}

// VariableAxis
//

VariableAxis::VariableAxis() {};

VariableAxis::VariableAxis(const std::vector<double>& bins)
{    
    _init_bins(bins.size());
    
    std::copy(bins.begin(), bins.end(), &(_bin_bounds[1]));
    
    _min = _bin_bounds[1];
    _max = _bin_bounds[_bins];
    
    assert(sane());
}

VariableAxis::VariableAxis(const VariableAxis & a)
{
    label = a.label;
}

void VariableAxis::_init_bins(const uint32_t bins) {
    _bins = bins;
    assert(_bins);

    _bin_bounds.reset(new double[_bins+2]);
    
    // Set under and overflow bounds
    _bin_bounds[0]       = -std::numeric_limits<double>::infinity();
    _bin_bounds[_bins+1] = +std::numeric_limits<double>::infinity();
    
    _bin_bounds_end = _bin_bounds.get() + _bins + 1;
}

VariableAxis::~VariableAxis() 
{
}

VariableAxis::VariableAxis(const pb::Axis & msg) {
    label = msg.label();
    
    _init_bins(msg.variable_bins_size());
    
    int i = 1;
    for (auto& value: msg.variable_bins())
        _bin_bounds[i++] = value;
    
    _min = _bin_bounds[1];
    _max = _bin_bounds[_bins];
    
    assert(sane());
}

unique<pb::Axis> VariableAxis::get_proto() {
    unique<pb::Axis> axis(new pb::Axis);
    axis->set_label(label);
    for (double const* x = _bin_bounds.get() + 1; x < _bin_bounds_end - 1; x++)
        axis->add_variable_bins(*x);
    return axis;
}

bool VariableAxis::sane() const
{
    return std::is_sorted(_bin_bounds.get(), _bin_bounds_end);
}

uint32_t VariableAxis::find_bin(const double& value) const
{
    // upper_bound does a binary search
    return std::upper_bound(_bin_bounds.get(), _bin_bounds_end, value) 
            - _bin_bounds.get() - 1;
}

};};
