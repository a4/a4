#ifndef A4_RESULTS_H
#define A4_RESULTS_H

#include <map>
#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include "H1.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)      
#define HIST1(RESULTS,NAME,NBIN,XMIN,XMAX) static Results::H1Ptr TOKENPASTE2(_TMPRES, __LINE__) = RESULTS->h1(NAME,NBIN,XMIN,XMAX); TOKENPASTE2(_TMPRES, __LINE__)

class Results
{
    public:
        typedef boost::shared_ptr<H1> H1Ptr;

        Results();
        virtual ~Results();

        virtual void add(const Results &);
        virtual void print(std::ostream &) const;
        virtual void print() const { print(std::cout); };

        H1Ptr h1(const char * name);
        H1Ptr h1(const char * name, const uint32_t &bins, const double &min, const double &max);

    private:
        typedef std::map<const char *, H1Ptr> HMap;
        HMap _h1;

};

#endif
