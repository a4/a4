#ifndef A4_H1_H
#define A4_H1_H

#include <iosfwd>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <inttypes.h>
#include <a4/axis.h>
#include <a4/streamable.h>
#include <a4/binned_data.h>

class H1 : public streamable, public BinnedData
{
    public:
        H1();
        H1(const H1 &);
        H1(Message &);
        ~H1();

        H1 & operator()(const uint32_t &bins, const double &min, const double &max);


        void fill(const double &, const double &weight = 1);
        BinnedData & __add__(const BinnedData &);
        BinnedData & __mul__(const double &);

        uint64_t entries() const {return _entries;};
        uint64_t bins() const {return _axis.bins();};
        double integral() const;

        const Axis & x() const {return _axis;};

        void print(std::ostream &) const;

        typedef boost::shared_array<double> DataPtr;
        const DataPtr data() const {return _data;}; //TODO: only for copyin into TH1D
        const DataPtr weights_squared() const {return _weights_squared;}; //TODO: only for copyin into TH1D

        virtual MessagePtr get_message();

    private:
        // Prevent copying by assignment
        H1 &operator =(const H1 &);

        Axis _axis;
        DataPtr _data;
        DataPtr _weights_squared;
        uint64_t _entries;
        bool _initialized;
};

typedef boost::shared_ptr<H1> H1Ptr;

std::ostream &operator<<(std::ostream &, const H1 &);

#endif
