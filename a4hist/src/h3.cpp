
#include <iomanip>
#include <iostream>
#include <string.h>

#include "a4/axis.h"
#include "a4/histogram.h"

#include "Histograms.pb.h"

namespace a4{ namespace hist{

using namespace std;

H3::H3() :
    _entries(0)
{
    _initializations_remaining = 3;
}

void H3::constructor(const uint32_t &bins, const double &min, const double &max, const char * _label) {
    if (_initializations_remaining == 2) {
        _x_axis.reset(new SimpleAxis(bins, min, max));
        _x_axis->label = _label;
    } else if (_initializations_remaining == 1)  {
        _y_axis.reset(new SimpleAxis(bins, min, max));
        _y_axis->label = _label;
    } else {
        _z_axis.reset(new SimpleAxis(bins, min, max));
        _z_axis->label = _label;
        _entries = 0;
        const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
        _data.reset(new double[total_bins]());
    }
}

void H3::constructor(const std::vector<double>& bins, const char* label) {
    if (_initializations_remaining == 2) {
        _x_axis.reset(new VariableAxis(bins));
        _x_axis->label = label;
    } else if (_initializations_remaining == 1)  {
        _y_axis.reset(new VariableAxis(bins));
        _y_axis->label = label;
    } else {
        _z_axis.reset(new VariableAxis(bins));
        _z_axis->label = label;
        
        _entries = 0;
        const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
        _data.reset(new double[total_bins]());
    }
}

H3::H3(const H3 & h): 
    title(h.title),
    _entries(h._entries)
{
    _x_axis = Axis::from_proto(*h._x_axis->get_proto());
    _y_axis = Axis::from_proto(*h._y_axis->get_proto());
    _z_axis = Axis::from_proto(*h._z_axis->get_proto());
    
    const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
    _data.reset(new double[total_bins]);
    memcpy(_data.get(), h._data.get(), total_bins * sizeof(double));
    if (h._weights_squared) {
        _weights_squared.reset(new double[total_bins]);
        memcpy(_weights_squared.get(), h._weights_squared.get(), total_bins * sizeof(double));
    }
    _initializations_remaining = 0;
}

H3::~H3()
{
};

H3 & H3::__mul__(const double & w) {
    const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
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
void H3::to_pb(bool blank_pb) {
    if (!blank_pb) pb.reset(new pb::H3());
    pb->mutable_x()->CopyFrom(*_x_axis->get_proto());
    pb->mutable_y()->CopyFrom(*_y_axis->get_proto());
    pb->mutable_z()->CopyFrom(*_z_axis->get_proto());
    const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
    for(uint32_t i = 0; i < total_bins; i++) pb->add_data(_data[i]);
    if (_weights_squared)
        for(uint32_t i = 0; i < total_bins; i++) pb->add_weights_squared(_weights_squared[i]);
    pb->set_entries(_entries);
    pb->set_title(title);
};

void H3::from_pb() {
    _x_axis = Axis::from_proto(pb->x());
    _y_axis = Axis::from_proto(pb->y());
    _z_axis = Axis::from_proto(pb->z());
    _entries = pb->entries();
    const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
    _data.reset(new double[total_bins]);
    for (uint32_t i = 0; i < total_bins; i++) _data[i] = pb->data(i);
    if (pb->weights_squared_size() > 0) {
        _weights_squared.reset(new double[total_bins]);
        for (uint32_t i = 0; i < total_bins; i++) _weights_squared[i] = pb->weights_squared(i);
    }
    title = pb->title();
};

H3 & H3::operator+=(const H3 &other) {
    return this->__add__(other);
};

double H3::integral() const
{
    double integral = 0;
    const int skip_x = _x_axis->bins() + 2;
    const int skip_y = _y_axis->bins() + 2;
    for(uint32_t binx = 1, binsx = binx + _x_axis->bins(); binsx > binx; ++binx) 
        for(uint32_t biny = 1, binys = biny + _y_axis->bins(); binys > biny; ++biny)
            for(uint32_t binz = 1, binzs = binz + _z_axis->bins(); binzs > binz; ++binz)
                integral += *(_data.get() + binx + skip_x*(biny + skip_y*binz));

    return integral;
}

void H3::print(std::ostream &out) const
{
    if (!_data)
    {
        cerr << "Histogram is not initialized" << endl;
        return;
    }

    if (!title.empty()) out << "Title: " << title << endl;
    out << "Entries: " << entries() << " Integral: " << integral() << endl;

    const int skip_x = _x_axis->bins() + 2;
    const int skip_y = _y_axis->bins() + 2;
    for(uint32_t binx = 1, binsx = binx + _x_axis->bins(); binsx > binx; ++binx) 
        for(uint32_t biny = 1, binys = biny + _y_axis->bins(); binys > biny; ++biny)
            for(uint32_t binz = 1, binzs = binz + _z_axis->bins(); binzs > binz; ++binz)
                out << "[" << setw(3) << binx << ", " << biny << ", " << binz << "]: " 
                    << setiosflags(ios::fixed) << setprecision(3) << *(_data.get() + binx + skip_x*(biny + skip_y*binz)) << endl;
}

H3 & H3::__add__(const H3 & source)
{
    const uint32_t total_bins = (_x_axis->bins() + 2)*(_y_axis->bins() + 2)*(_z_axis->bins() + 2);
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
ostream &operator<<(ostream &out, const H3 &hist)
{
    hist.print(out);

    return out;
}

};}; // namespace a4::hist;
