#ifndef A4_RESULTS_H
#define A4_RESULTS_H

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include "h1.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)      
#define HIST1(NAME,NBIN,XMIN,XMAX) static int TOKENPASTE2(_H1ID, __LINE__) = ++_histogram_fast_access_id; _hfast(TOKENPASTE2(_H1ID, __LINE__),NAME,NBIN,XMIN,XMAX)
#define HIST1_AS(V,NAME,NBIN,XMIN,XMAX) static int TOKENPASTE2(_H1ID, __LINE__) = ++_histogram_fast_access_id; H1Ptr V = _hfast(TOKENPASTE2(_H1ID, __LINE__),NAME,NBIN,XMIN,XMAX);

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
        typedef boost::shared_ptr<HMap> HMapPtr;
        HMapPtr _h1;

};

typedef boost::shared_ptr<Results> ResultsPtr;

#endif
