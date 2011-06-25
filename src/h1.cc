
#include <iomanip>
#include <iostream>
#include <string.h>

#include "a4/axis.h"
#include "a4/h1.h"

using namespace std;

H1::H1(const uint32_t &bins, const double &min, const double &max):
    _axis(bins, min, max),
    _entries(0)
{
    const uint32_t total_bins = bins + 2;
    _data.reset(new double[total_bins]());
}

H1::H1(const H1 & h): 
    _axis(h._axis),
    _entries(h._entries)
{
    const uint32_t total_bins = h.bins() + 2;
    _data.reset(new double[total_bins]);
    memcpy(_data.get(), h._data.get(), total_bins * sizeof(double));
}

H1::~H1()
{
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
    if (!_data)
        return;

    *(_data.get() + _axis.find_bin(x)) += weight;
    ++_entries;
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
        out << "[" << setw(3) << bin << "]: " << *(_data.get() + bin) << endl;
}

void H1::add(const H1 &source)
{
    for(uint32_t bin = 0, bins = _axis.bins() + 2; bins > bin; ++bin)
        *(_data.get() + bin) += *(source._data.get() + bin);
    _entries += source._entries;
}

// Helpers
//
ostream &operator<<(ostream &out, const H1 &hist)
{
    hist.print(out);

    return out;
}
