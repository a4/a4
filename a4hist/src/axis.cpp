#include <iostream>

#include "a4/axis.h"
#include "a4/hist/Histograms.pb.h"

using namespace std;
namespace a4{ namespace hist{

unique<Axis> Axis::from_proto(const pb::SimpleAxis & msg) {
    return unique<Axis>(new SimpleAxis(msg));
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

SimpleAxis::SimpleAxis(const pb::SimpleAxis & msg) {
    label = msg.label();
    _min = msg.min();
    _max = msg.max();
    _bins = msg.bins();
    _delta = _bins == 0 ? 0 : (_max - _min)/_bins;
};

unique<pb::SimpleAxis> SimpleAxis::get_proto() {
    unique<pb::SimpleAxis> axis(new pb::SimpleAxis);
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
ostream &operator<<(ostream &out, const Axis &axis)
{
    return out << axis.label << " - " << axis.bins() << " bins in range ["
        << axis.min() << "," << axis.max() << "]";
}

};};
