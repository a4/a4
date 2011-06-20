/**
 * Histogram Axis
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#include <iostream>

#include "Axis.h"

using namespace std;

Axis::Axis():
    _min(0),
    _max(0),
    _bins(0),
    _delta(0)
{
}

Axis::~Axis()
{
}

bool Axis::init(const uint32_t &bins, const double &min, const double &max)
{
    if (1 > bins ||
        min >= max)

        return false;

    _bins = bins;
    _min = min;
    _max = max;
    _delta = (max - min) / bins;

    return true;
}

double Axis::min() const
{
    return _min;
}

double Axis::max() const
{
    return _max;
}

uint32_t Axis::bins() const
{
    return _bins;
}

uint32_t Axis::findBin(const double &x) const
{
    if (!_bins)
        return 0;

    if (x < _min)
        return 0;

    if (x > _max)
        return 1 + _bins;

    return 1 + static_cast<int>((x - _min) / _delta);
}



// Helpers
//
ostream &operator<<(ostream &out, const Axis &axis)
{
    return out << axis.bins() << " bins in range ["
        << axis.min() << "," << axis.max() << "]";
}
