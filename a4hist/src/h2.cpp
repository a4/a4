
#include <iomanip>
#include <iostream>
#include <string.h>

#include "a4/axis.h"
#include "a4/histogram.h"

#include "Histograms.pb.h"

namespace a4{ namespace hist{

using namespace std;

H2::H2() :
    _x_axis(0, 0, 0),
    _y_axis(0, 0, 0),
    _entries(0),
    _initialized(false)
{}

H2 & H2::operator()(const uint32_t &xbins, const double &xmin, const double &xmax, const uint32_t &ybins, const double &ymin, const double &ymax) {
    if (_initialized) return *this;
    _initialized = true;
    _x_axis = Axis(xbins, xmin, xmax);
    _y_axis = Axis(ybins, ymin, ymax);
    _entries = 0;
    const uint32_t total_bins = (xbins + 2)*(ybins + 2);
    _data.reset(new double[total_bins]());
    return *this;
}

H2::H2(const H2 & h): 
    _x_axis(h._x_axis),
    _y_axis(h._y_axis),
    _entries(h._entries),
    _initialized(true)
{
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    _data.reset(new double[total_bins]);
    memcpy(_data.get(), h._data.get(), total_bins * sizeof(double));
    if (h._weights_squared) {
        _weights_squared.reset(new double[total_bins]);
        memcpy(_weights_squared.get(), h._weights_squared.get(), total_bins * sizeof(double));
    }
}

H2::~H2()
{
};

H2 & H2::__mul__(const double & w) {
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    for(uint32_t bin = 0, bins = total_bins; bins > bin; ++bin)
        *(_data.get() + bin) *= w;
    if (!_weights_squared) {
        _weights_squared.reset(new double[total_bins]);
        for(uint32_t i = 0; i < total_bins; i++) _weights_squared[i] = _data[i];
    }
    for(uint32_t bin = 0, bins = total_bins; bins > bin; ++bin)
        *(_weights_squared.get() + bin) *= w*w;
    _entries *= w;
    return *this;
}

// Implements StorableAs
void H2::to_pb(bool blank_pb) {
    if (!blank_pb) pb.reset(new pb::H2());
    pb->mutable_x()->set_bins(_x_axis.bins());
    pb->mutable_x()->set_min(_x_axis.min());
    pb->mutable_x()->set_max(_x_axis.max());
    pb->mutable_y()->set_bins(_y_axis.bins());
    pb->mutable_y()->set_min(_y_axis.min());
    pb->mutable_y()->set_max(_y_axis.max());
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    for(uint32_t i = 0; i < total_bins; i++) pb->add_data(_data[i]);
    if (_weights_squared)
        for(uint32_t i = 0; i < total_bins; i++) pb->add_weights_squared(_weights_squared[i]);
    pb->set_entries(_entries);
};

void H2::from_pb() {
    _x_axis = Axis(pb->x().bins(), pb->x().min(), pb->x().max());
    _y_axis = Axis(pb->y().bins(), pb->y().min(), pb->y().max());
    _entries = pb->entries();
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    _data.reset(new double[total_bins]);
    for (int i = 0; i < total_bins; i++) _data[i] = pb->data(i);
    if (pb->weights_squared_size() > 0) {
        _weights_squared.reset(new double[total_bins]);
        for (int i = 0; i < total_bins; i++) _weights_squared[i] = pb->weights_squared(i);
    }
};

H2 & H2::operator+=(const H2 &other) {
    return this->__add__(other);
};

double H2::integral() const
{
    double integral = 0;
    const int skip = _x_axis.bins() + 2;
    for(uint32_t bin = 1, bins = bin + _x_axis.bins(); bins > bin; ++bin) 
        for(uint32_t ybin = 1, ybins = ybin + _y_axis.bins(); ybins > ybin; ++ybin)
            integral += *(_data.get() + ybin*skip + bin);

    return integral;
}

void H2::fill(const double &x, const double & y, const double &weight)
{
    int binx = _x_axis.find_bin(x);
    int biny = _y_axis.find_bin(y);

    const int skip = _x_axis.bins() + 2;
    *(_data.get() + binx + biny*skip) += weight;
    ++_entries;

    if (_weights_squared) {
        *(_weights_squared.get() + binx + biny*skip) += weight*weight;
    } else if (weight != 1.0) {
        const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
        _weights_squared.reset(new double[total_bins]);
        for(uint32_t i = 0; i < total_bins; i++)
            _weights_squared[i] = _data[i];
        *(_weights_squared.get() + binx + biny*skip) += weight*weight;
    }
}

void H2::print(std::ostream &out) const
{
    if (!_data)
    {
        cerr << "Histogram is not initialized" << endl;
        return;
    }

    out << "X Axis: " << _x_axis << endl;
    out << "Y Axis: " << _y_axis << endl;
    out << "Entries: " << entries() << " Integral: " << integral() << endl;

    const int skip = _x_axis.bins() + 2;
    for(uint32_t bin = 1, bins = bin + _x_axis.bins(); bins > bin; ++bin) 
        for(uint32_t ybin = 1, ybins = ybin + _y_axis.bins(); ybins > ybin; ++ybin)
            out << "[" << setw(3) << bin << "]: " << setiosflags(ios::fixed) << setprecision(3) << *(_data.get() + ybin*skip + bin) << endl;
}

H2 & H2::__add__(const H2 & source)
{
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    for(uint32_t bin = 0, bins = total_bins; bins > bin; ++bin)
        *(_data.get() + bin) += *(source._data.get() + bin);
    if (_weights_squared)
        for(uint32_t bin = 0, bins = total_bins; bins > bin; ++bin)
            *(_weights_squared.get() + bin) += *(source._weights_squared.get() + bin);
    _entries += source._entries;
    return *this;
}

// Helpers
//
ostream &operator<<(ostream &out, const H2 &hist)
{
    hist.print(out);

    return out;
}

};}; // namespace a4::hist;
