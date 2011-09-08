#ifndef A4_RESULTS_H
#define A4_RESULTS_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "a4/interfaces.h"
#include "a4/streamable.h"
#include "a4/objectstore.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)      

#define HIST1(NAME,NBIN,XMIN,XMAX) _results->get<H1>(__LINE__, NAME)(NBIN,XMIN,XMAX)
#define HIST1_AS(V,NAME,NBIN,XMIN,XMAX) H1Ptr V = _results->get<H1>(__LINE__, NAME)(NBIN,XMIN,XMAX);
#define HIST1_FILL(NAME,NBIN,XMIN,XMAX,FV,W) _results->get<H1>(__LINE__,NAME)(NBIN,XMIN,XMAX).fill(FV,W);
#define HIST2(NAME,NBIN,XMIN,XMAX,YBIN,YMIN,YMAX) _results->get<H2>(__LINE__, NAME)(NBIN,XMIN,XMAX,YBIN,YMIN,YMAX)
#define HIST2_AS(V,NAME,NBIN,XMIN,XMAX) H2Ptr V = _results->get<H2>(__LINE__, NAME)(NBIN,XMIN,XMAX,YBIN,YMIN,YMAX);
#define HIST2_FILL(NAME,NBIN,XMIN,XMAX,YBIN,YMIN,YMAX,FVX,FVY,W) _results->get<H2>(__LINE__,NAME)(NBIN,XMIN,XMAX,YBIN,YMIN,YMAX).fill(FVX,FVY,W);

#define CUTFLOW(V,NAME) Cutflow & V = _results->get<Cutflow>(__LINE__, NAME);
#define PASSED_CUT(cf,cut_name,w) {static int _CutID = ++Cutflow::_fast_access_id; cf.fill(_CutID, cut_name, w);}

using std::string;

class Results;
typedef boost::shared_ptr<Results> ResultsPtr;

class Results : public Printable, public Addable, public Scalable, public ObjectStore<Printable>
{
    public:
        virtual Results & __add__(const Addable &);
        virtual Results & __mul__(const double &);
        virtual Results * clone() const;
        std::string __repr__();
        std::string __str__();
        boost::shared_ptr<MetaData> metadata;

        void to_file(std::string fn);
        static std::vector<Results *> from_file(std::string fn);
};
typedef boost::shared_ptr<Results> ResultsPtr;

#endif
