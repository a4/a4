#ifndef RESULTS_H
#define RESULTS_H

#include <map>
#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include "H1.h"
            
class Results
{
    public:
        typedef boost::shared_ptr<H1> H1Ptr;

        Results();
        virtual ~Results();

        virtual void add(const Results &);
        virtual void print(std::ostream &) const;
        virtual void print() const { print(std::cout); };

        H1Ptr h1(std::string name);
        H1Ptr h1(std::string name, const uint32_t &bins, const double &min, const double &max);

    private:
        typedef std::map<std::string, H1Ptr> HMap;
        HMap _h1;
};

#endif
