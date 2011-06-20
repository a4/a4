/**
 * 1D Histogram
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef H1_H
#define H1_H

#include <iosfwd>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <inttypes.h>

class Axis;
class TH1;

class H1
{
    public:
        typedef boost::shared_ptr<TH1> TH1Ptr;

        H1(const uint32_t &bins, const double &min, const double &max);
        ~H1();

        uint64_t entries() const;
        double integral() const;

        void fill(const double &, const double &weight = 1);
        void print(std::ostream &) const;


        void add(const H1 &);

    private:
        // Temporarily prevent copying
        //
        H1(const H1 &);
        H1 &operator =(const H1 &);

        typedef boost::shared_ptr<Axis> AxisPtr;
        typedef boost::shared_array<double> DataPtr;

        AxisPtr _axis;
        DataPtr _data;

        uint64_t _entries;
};

std::ostream &operator<<(std::ostream &, const H1 &);

#endif
