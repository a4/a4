#ifndef _A4_BINNEDDATA_H
#define _A4_BINNEDDATA_H

class BinnedData {
    public:
        virtual BinnedData & __add__(const BinnedData &) = 0;
        virtual BinnedData & __mul__(const double &) = 0;

        BinnedData & operator*=(const double & rhs) { return __mul__(rhs); };
        BinnedData & operator+=(const BinnedData & rhs) { return __add__(rhs); };
};

#endif
