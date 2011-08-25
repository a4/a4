#ifndef _A4_RESULTTYPE_H
#define _A4_RESULTTYPE_H

#include <a4/streamable.h>

class ResultType : public Streamable {
    public:
        ResultType() : initialized(false) {};

        virtual ResultType & __add__(const ResultType &) = 0;
        virtual ResultType & __mul__(const double &) { return *this; };

        ResultType & operator*=(const double & rhs) { return __mul__(rhs); };
        ResultType & operator+=(const ResultType & rhs) { return __add__(rhs); };

    protected:
        bool initialized;
};

#endif
