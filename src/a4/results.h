#ifndef A4_RESULTS_H
#define A4_RESULTS_H

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include "H1.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)      
#define HIST1(RESULTS,NAME,NBIN,XMIN,XMAX) static H1Ptr TOKENPASTE2(_TMPRES, __LINE__) = RESULTS->h1(NAME,NBIN,XMIN,XMAX); TOKENPASTE2(_TMPRES, __LINE__)

using std::string;

class Results
{
    public:
        Results();
        virtual ~Results();

        virtual void add(const Results &);
        virtual void print(std::ostream &) const;
        virtual void print() const { print(std::cout); };
        virtual std::vector<std::string> names() const;

        H1Ptr h1(string name);
        H1Ptr h1(string name, const uint32_t &bins, const double &min, const double &max);

    private:
        typedef std::map<string, H1Ptr> HMap;
        HMap _h1;

};

typedef boost::shared_ptr<Results> ResultsPtr;

#endif
