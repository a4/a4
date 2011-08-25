#ifndef _A4_RESULTTYPE_H
#define _A4_RESULTTYPE_H

#include <google/protobuf/message.h>

class ResultType {
    public:
        ResultType() = 0;

        virtual void from_message(google::protobuf::Message &) = 0;
        virtual google::protobuf::Message * get_message() = 0;

        virtual ResultType & __add__(const ResultType &) = 0;
        virtual ResultType & __mul__(const double &) { return *this; };

        ResultType & operator*=(const double & rhs) { return __mul__(rhs); };
        ResultType & operator+=(const ResultType & rhs) { return __add__(rhs); };
};

#endif
