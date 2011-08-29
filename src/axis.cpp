#include <iostream>

#include "a4/axis.h"
#include "a4/proto/io/Histograms.pb.h"

using namespace std;

Axis::Axis() {};

Axis::Axis(const uint32_t &bins, const double &min, const double &max):
    _min(min),
    _max(max),
    _bins(bins),
    _delta(bins == 0 ? 0 : (max - min)/bins)
{   
};

Axis::Axis(const Axis & a):
    _min(a._min),
    _max(a._max),
    _bins(a._bins),
    _delta(a._delta)
{
}

Axis::~Axis() 
{
};

void Axis::from_message(google::protobuf::Message & m) {
    a4::io::Axis * msg = dynamic_cast<a4::io::Axis*>(&m);
    _min = msg->min();
    _max = msg->max();
    _bins = msg->bins();
    _delta = _bins == 0 ? 0 : (_max - _min)/_bins;
};

MessagePtr Axis::get_message() {
    boost::shared_ptr<a4::io::Axis> axis(new a4::io::Axis);
    axis->set_bins(_bins);
    axis->set_min(_min);
    axis->set_max(_max);
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
    return out << axis.bins() << " bins in range ["
        << axis.min() << "," << axis.max() << "]";
}
