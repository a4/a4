
#include <iomanip>
#include <iostream>
#include <string.h>
#include <vector>

#include <a4/axis.h>
#include <a4/histogram.h>

#include <a4/hist/Histograms.pb.h>

namespace a4{ namespace hist{

using namespace std;

H1::H1() :
    _entries(0)
{
    _initializations_remaining = 1;
}

void H1::bin_init() {
    _entries = 0;
    const uint32_t total_bins = _axis->bins() + 2;
    _data.reset(new double[total_bins]());
}

void H1::constructor(const uint32_t &bins, const double &min, const double &max, const char* label) {
    _axis.reset(new SimpleAxis(bins, min, max));
    _axis->label = label;
    bin_init();
}

void H1::constructor(const std::vector<double>& bins, const char* label) {
    _axis.reset(new VariableAxis(bins));
    _axis->label = label;
    bin_init();
}

H1::H1(const H1& h):
    title(h.title),
    _entries(h._entries)
{
    _axis = Axis::from_proto(*h._axis->get_proto());
    const uint32_t total_bins = h.bins() + 2;
    _data.reset(new double[total_bins]);
    memcpy(_data.get(), h._data.get(), total_bins * sizeof(double));
    if (h._weights_squared) {
        _weights_squared.reset(new double[total_bins]);
        memcpy(_weights_squared.get(), h._weights_squared.get(), total_bins * sizeof(double));
    }
    _initializations_remaining = 0;
}

H1::~H1()
{
}

void H1::ensure_weights() {
    const uint32_t total_bins = _axis->bins() + 2;
    if (!_weights_squared) {
        _weights_squared.reset(new double[total_bins]);
        for(uint32_t i = 0; i < total_bins; i++) _weights_squared[i] = _data[i];
    }
}

H1& H1::__mul__(const double& w) {
    const uint32_t total_bins = _axis->bins() + 2;
    ensure_weights();
    for(uint32_t bin = 0, bins = total_bins; bins > bin; ++bin)
        *(_data.get() + bin) *= w;
    for(uint32_t bin = 0, bins = total_bins; bins > bin; ++bin) {
        *(_weights_squared.get() + bin) *= w*w;
    }
    _entries *= w;
    return *this;
}


// Implements StorableAs
void H1::to_pb(bool blank_pb) {
    if (!blank_pb) pb.reset(new pb::H1());
    pb->mutable_x()->CopyFrom(*_axis->get_proto());
    const uint32_t total_bins = _axis->bins() + 2;
    for(uint32_t i = 0; i < total_bins; i++) pb->add_data(_data[i]);
    if (_weights_squared)
        for(uint32_t i = 0; i < total_bins; i++) pb->add_weights_squared(_weights_squared[i]);
    pb->set_entries(_entries);
    pb->set_title(title);
};

void H1::from_pb() {
    _axis = Axis::from_proto(pb->x());
    _entries = pb->entries();
    const uint32_t total_bins = _axis->bins() + 2;
    _data.reset(new double[total_bins]);
    for (uint32_t i = 0; i < total_bins; i++) _data[i] = pb->data(i);
    if (pb->weights_squared_size() > 0) {
        _weights_squared.reset(new double[total_bins]);
        for (uint32_t i = 0; i < total_bins; i++) _weights_squared[i] = pb->weights_squared(i);
    }
    title = pb->title();
};

H1& H1::operator+=(const H1 &other) {
    return this->__add__(other);
};

double H1::integral() const
{
    double integral = 0;
    for(uint32_t bin = 1, bins = bin + _axis->bins(); bins > bin; ++bin)
        integral += *(_data.get() + bin);

    return integral;
}

void H1::print(std::ostream &out) const
{
    if (!_data)
    {
        ERROR("Histogram is not initialized");
        return;
    }

    if (!title.empty()) out << "Title: " << title << endl;
    out << "Entries: " << entries() << " Integral: " << int(integral()) << endl;

    for(uint32_t bin = 0, bins = _axis->bins() + 2; bins > bin; ++bin)
        //out << "[" << setw(3) << bin << "]: " << *(_data.get() + bin) << endl;
        out << "[" << setw(3) << bin << "]: " << setiosflags(ios::fixed) << setprecision(3) << *(_data.get() + bin) << endl;
}

H1& H1::__add__(const H1& source)
{
    for(uint32_t bin = 0, bins = _axis->bins() + 2; bins > bin; ++bin)
        *(_data.get() + bin) += *(source._data.get() + bin);
    if (source._weights_squared) {
        ensure_weights();
        for(uint32_t bin = 0, bins = _axis->bins() + 2; bins > bin; ++bin)
            *(_weights_squared.get() + bin) += *(source._weights_squared.get() + bin);
    } else if (_weights_squared) {
        for(uint32_t bin = 0, bins = _axis->bins() + 2; bins > bin; ++bin)
            *(_weights_squared.get() + bin) += *(source._data.get() + bin);
    }
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
