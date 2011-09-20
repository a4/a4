#ifndef A4_H2_H
#define A4_H2_H

#include <iosfwd>
#include <string>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <inttypes.h>
#include <a4/axis.h>
#include <a4/result_type.h>

class H2 : public ResultType
{
    public:
        H2();
        H2(const H2 &);
        H2(Message &);
        ~H2();

        virtual void from_message(google::protobuf::Message & m);
        virtual MessagePtr get_message();

        H2 & operator()(const uint32_t &binsx, const double &minx, const double &maxx, const uint32_t &binsy, const double &miny, const double &maxy);


        void fill(const double &, const double &, const double &weight = 1);
        ResultType & __add__(const ResultType &);
        ResultType & __mul__(const double &);

        uint64_t entries() const {return _entries;};
        double integral() const;

        const Axis & x() const {return _x_axis;};
        const Axis & y() const {return _y_axis;};

        void print(std::ostream &) const;

        typedef boost::shared_array<double> DataPtr;
        const DataPtr data() const {return _data;}; //TODO: only for copyin into TH2D
        const DataPtr weights_squared() const {return _weights_squared;}; //TODO: only for copyin into TH1D


    private:
        // Prevent copying by assignment
        H2 &operator =(const H2 &);

        Axis _x_axis;
        Axis _y_axis;
        DataPtr _data;
        DataPtr _weights_squared;
        uint64_t _entries;
        bool _initialized;
};

typedef boost::shared_ptr<H2> H2Ptr;

std::ostream &operator<<(std::ostream &, const H2 &);

#endif
