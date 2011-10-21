#include <iostream>

#include "a4/axis.h"
#include "a4/hist/Histograms.pb.h"

using namespace std;
namespace a4{ namespace hist{

Axis::Axis() {};

Axis::Axis(const uint32_t &bins, const double &min, const double &max):
    label(""),
    _min(min),
    _max(max),
    _bins(bins),
    _delta(bins == 0 ? 0 : (max - min)/bins)
{   
};

Axis::Axis(const Axis & a):
    label(a.label),
    _min(a._min),
    _max(a._max),
    _bins(a._bins),
    _delta(a._delta)
{
}

Axis::~Axis() 
{
};

Axis::Axis(const pb::Axis & msg) {
    label = msg.label();
    _min = msg.min();
    _max = msg.max();
    _bins = msg.bins();
    _delta = _bins == 0 ? 0 : (_max - _min)/_bins;
};

unique<pb::Axis> Axis::get_proto() {
    unique<pb::Axis> axis(new pb::Axis);
    axis->set_bins(_bins);
    axis->set_min(_min);
    axis->set_max(_max);
    axis->set_label(label);
    return axis;
};

bool Axis::sane() const
{
    return (_bins > 0 && _min <= _max);
}

uint32_t Axis::find_bin(const double &x) const
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
