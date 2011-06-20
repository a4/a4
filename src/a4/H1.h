/**
 * 1D Histogram
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef A4_H1_H
#define A4_H1_H

#include <iosfwd>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <inttypes.h>
#include <a4/axis.h>


class H1
{
    public:
        H1(const uint32_t &bins, const double &min, const double &max);
        ~H1();

        uint64_t entries() const;
        uint64_t n_bins() const;
        double integral() const;

        void fill(const double &, const double &weight = 1);
        void print(std::ostream &) const;

        void add(const H1 &);


        typedef boost::shared_ptr<Axis> AxisPtr;
        typedef boost::shared_array<double> DataPtr;

        AxisPtr x() {return _axis;};
        DataPtr data() {return _data;}; //TODO: only for copyin into TH1D

    private:
        // Temporarily prevent copying
        //
        AxisPtr _axis;
        DataPtr _data;

        uint64_t _entries;

        H1(const H1 &);
        H1 &operator =(const H1 &);

};

typedef boost::shared_ptr<H1> H1Ptr;



std::ostream &operator<<(std::ostream &, const H1 &);

#endif
