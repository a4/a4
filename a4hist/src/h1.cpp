
#include <iomanip>
#include <iostream>
#include <string.h>

#include <boost/foreach.hpp>

#include "a4/axis.h"
#include "a4/histogram.h"

#include "Histograms.pb.h"

namespace a4{ namespace hist{

using namespace std;

void H1::constructor(const uint32_t &bins, const double &min, const double &max) {
    _axis = Axis(bins, min, max);
    _entries = 0;
    const uint32_t total_bins = bins + 2;
    _data.reset(new double[total_bins]());
}

H1::H1() :
    _axis(0, 0, 0),
    _entries(0)
{}

H1::H1(const H1 & h): 
    _axis(h._axis),
    _entries(h._entries),
    _initialized(true)
{
    const uint32_t total_bins = h.bins() + 2;
    _data.reset(new double[total_bins]);
    memcpy(_data.get(), h._data.get(), total_bins * sizeof(double));
    if (h._weights_squared) {
        _weights_squared.reset(new double[total_bins]);
        memcpy(_weights_squared.get(), h._weights_squared.get(), total_bins * sizeof(double));
    }
}

H1::~H1()
{
}

H1 & H1::__mul__(const double & w) {
    const uint32_t total_bins = _axis.bins() + 2;
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
void H1::to_pb(bool blank_pb) {
    if (!blank_pb) pb.reset(new pb::H1());
    pb->mutable_x()->set_bins(_axis.bins());
    pb->mutable_x()->set_min(_axis.min());
    pb->mutable_x()->set_max(_axis.max());
    const uint32_t total_bins = _axis.bins() + 2;
    for(uint32_t i = 0; i < total_bins; i++) pb->add_data(_data[i]);
    if (_weights_squared)
        for(uint32_t i = 0; i < total_bins; i++) pb->add_weights_squared(_weights_squared[i]);
    pb->set_entries(_entries);
};

void H1::from_pb() {
    _axis = Axis(pb->x().bins(), pb->x().min(), pb->x().max());
    _entries = pb->entries();
    const uint32_t total_bins = _axis.bins() + 2;
    _data.reset(new double[total_bins]);
    for (int i = 0; i < total_bins; i++) _data[i] = pb->data(i);
    if (pb->weights_squared_size() > 0) {
        _weights_squared.reset(new double[total_bins]);
        for (int i = 0; i < total_bins; i++) _weights_squared[i] = pb->weights_squared(i);
    }
};

 H1 & H1::operator+=(const H1 &other) {
    return this->__add__(other);
};

double H1::integral() const
{
    double integral = 0;
    for(uint32_t bin = 1, bins = bin + _axis.bins(); bins > bin; ++bin)
        integral += *(_data.get() + bin);

    return integral;
}

void H1::fill(const double &x, const double &weight)
{
    int bin = _axis.find_bin(x);
    *(_data.get() + bin) += weight;
    ++_entries;

    if (_weights_squared) {
        *(_weights_squared.get() + bin) += weight*weight;
    } else if (weight != 1.0) {
        const uint32_t total_bins = _axis.bins() + 2;
        _weights_squared.reset(new double[total_bins]);
        for(uint32_t i = 0; i < total_bins; i++)
            _weights_squared[i] = _data[i];
        *(_weights_squared.get() + bin) += weight*weight;
    }
}

void H1::print(std::ostream &out) const
{
    if (!_data)
    {
        cerr << "Histogram is not initialized" << endl;
        return;
    }

    out << "X Axis: " << _axis << endl;
    out << "Entries: " << entries() << " Integral: " << integral() << endl;

    for(uint32_t bin = 0, bins = _axis.bins() + 2; bins > bin; ++bin)
        //out << "[" << setw(3) << bin << "]: " << *(_data.get() + bin) << endl;
        out << "[" << setw(3) << bin << "]: " << setiosflags(ios::fixed) << setprecision(3) << *(_data.get() + bin) << endl;
}

H1 & H1::__add__(const H1 & source)
{
    for(uint32_t bin = 0, bins = _axis.bins() + 2; bins > bin; ++bin)
        *(_data.get() + bin) += *(source._data.get() + bin);
    if (_weights_squared)
        for(uint32_t bin = 0, bins = _axis.bins() + 2; bins > bin; ++bin)
            *(_weights_squared.get() + bin) += *(source._weights_squared.get() + bin);
    _entries += source._entries;
    return *this;
}

// Helpers
//
ostream &operator<<(ostream &out, const H1 &hist)
{
    hist.print(out);

    return out;
}

};}; //namespace a4::hist;
