#ifndef A4_H1_H
#define A4_H1_H

#include <iosfwd>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <inttypes.h>
#include <a4/axis.h>

class Table
{
    public:
        Table(const uint32_t &lines);
        Table(const Table &);
        ~Table();

        void fill(const int &line, const double &weight = 1);
        void add(const Table &);

        uint64_t entries() const {return _entries;};
        uint64_t bins() const {return _axis.bins();};
        double integral() const;

        const Axis & x() const {return _axis;};

        void print(std::ostream &) const;

        typedef boost::shared_array<double> DataPtr;
        const DataPtr data() const {return _data;}; //TODO: only for copyin into TH1D

    private:
        // Prevent copying by assignment
        H1 &operator =(const H1 &);

        Axis _axis;
        DataPtr _data;
        uint64_t _entries;
};

typedef boost::shared_ptr<H1> H1Ptr;

std::ostream &operator<<(std::ostream &, const H1 &);

#endif
