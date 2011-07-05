#ifndef A4_H2_H
#define A4_H2_H

#include <iosfwd>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <inttypes.h>
#include <a4/axis.h>

class H2
{
    public:
        H2(const uint32_t &binsx, const double &minx, const double &maxx, const uint32_t &binsy, const double &miny, const double &maxy);
        H2(const H2 &);
        ~H2();

        void fill(const double &, const double &, const double &weight = 1);
        void add(const H2 &);

        uint64_t entries() const {return _entries;};
        double integral() const;

        const Axis & x() const {return _x_axis;};
        const Axis & y() const {return _y_axis;};

        void print(std::ostream &) const;

        typedef boost::shared_array<double> DataPtr;
        const DataPtr data() const {return _data;}; //TODO: only for copyin into TH2D

    private:
        // Prevent copying by assignment
        H2 &operator =(const H2 &);

        Axis _x_axis;
        Axis _y_axis;
        DataPtr _data;
        uint64_t _entries;
};

typedef boost::shared_ptr<H2> H2Ptr;

std::ostream &operator<<(std::ostream &, const H2 &);

#endif
