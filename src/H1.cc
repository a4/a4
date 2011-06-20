/**
 * 1D Histogram
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#include <iomanip>
#include <iostream>
#include <string.h>

#include "Axis.h"
#include "H1.h"

using namespace std;

H1::H1(const uint32_t &bins, const double &min, const double &max):
    _entries(0)
{
    _axis.reset(new Axis());
    if (_axis->init(bins, min, max))
    {
        const uint32_t total_bins = bins + 2;

        _data.reset(new double[total_bins]);
        memset(_data.get(), 0, total_bins * sizeof(double));
    }
}

H1::~H1()
{
}

uint64_t H1::entries() const
{
    return _entries;
}

double H1::integral() const
{
    double integral = 0;
    for(uint32_t bin = 1, bins = bin + _axis->bins();
        bins > bin;
        ++bin)

        integral += *(_data.get() + bin);

    return integral;
}

void H1::fill(const double &x, const double &weight)
{
    if (!_data)
        return;

    *(_data.get() + _axis->findBin(x)) += weight;
    ++_entries;
}

void H1::print(std::ostream &out) const
{
    if (!_data)
    {
        cerr << "Histogram is not initialized" << endl;

        return;
    }

    out << "X Axis: " << *_axis << endl;
    out << "Entries: " << entries() << " Integral: " << integral() << endl;

    for(uint32_t bin = 0, bins = _axis->bins() + 2; bins > bin; ++bin)
        out << "[" << setw(3) << bin << "]: " << *(_data.get() + bin) << endl;
}

void H1::add(const H1 &source)
{
    for(uint32_t bin = 0, bins = _axis->bins() + 2; bins > bin; ++bin)
        *(_data.get() + bin) += *(source._data.get() + bin);
}



// Helpers
//
ostream &operator<<(ostream &out, const H1 &hist)
{
    hist.print(out);

    return out;
}
