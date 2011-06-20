/**
 * Histogram Axis
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef AXIS_H
#define AXIS_H
#include <iosfwd>
#include <inttypes.h>

class Axis
{
    public:
        Axis();
        ~Axis();

        bool init(const uint32_t &bins, const double &min, const double &max);

        double min() const;
        double max() const;
        uint32_t bins() const;
        uint32_t findBin(const double &x) const;

    private:
        double _min;
        double _max;
        uint32_t _bins;
        double _delta;
};

std::ostream &operator<<(std::ostream &, const Axis &);

#endif
