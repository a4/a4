
#include <iomanip>
#include <iostream>
#include <string.h>

#include <boost/foreach.hpp>

#include "a4/axis.h"
#include "a4/h1.h"

#include "a4/proto/io/Histograms.pb.h"

using namespace std;


H1 & H1::operator()(const uint32_t &bins, const double &min, const double &max) {
    if (_initialized) return *this;
    _initialized = true;
    _axis = Axis(bins, min, max);
    _entries = 0;
    const uint32_t total_bins = bins + 2;
    _data.reset(new double[total_bins]());
    return *this;
}

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

ResultType & H1::__mul__(const double & w) {
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

MessagePtr H1::get_message() {
    boost::shared_ptr<a4::io::Histogram1> h1(new a4::io::Histogram1);
    h1->mutable_x()->set_bins(_axis.bins());
    h1->mutable_x()->set_min(_axis.min());
    h1->mutable_x()->set_max(_axis.max());
    const uint32_t total_bins = _axis.bins() + 2;
    for(uint32_t i = 0; i < total_bins; i++) h1->add_data(_data[i]);
    if (_weights_squared)
        for(uint32_t i = 0; i < total_bins; i++) h1->add_weights_squared(_weights_squared[i]);
    h1->set_entries(_entries);
    return h1;
}

H1::H1(Message& m) : _axis(0,0,0) {
    a4::io::Histogram1 * msg = dynamic_cast<a4::io::Histogram1*>(&m);
    _axis = Axis(msg->x().bins(), msg->x().min(), msg->x().max());
    _entries = msg->entries();
    const uint32_t total_bins = _axis.bins() + 2;
    _data.reset(new double[total_bins]);
    for (int i = 0; i < total_bins; i++) _data[i] = msg->data(i);
    if (msg->weights_squared_size() > 0) {
        _weights_squared.reset(new double[total_bins]);
        for (int i = 0; i < total_bins; i++) _weights_squared[i] = msg->weights_squared(i);
    }
}

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

ResultType & H1::__add__(const ResultType &_source)
{
    const H1 & source = dynamic_cast<const H1&>(_source);
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
