#ifndef A4_RESULTS_H
#define A4_RESULTS_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "a4/h1.h"
#include "a4/h2.h"
#include "a4/cutflow.h"
#include "a4/result_type.h"
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

class Results : public ResultType, public ObjectStore<ResultType>
{
    public:
        virtual void from_message(google::protobuf::Message &);
        virtual MessagePtr get_message();

        virtual ~Results();

        const string get_title() {return title;};
        void set_title(const string &t) { title = t;};
        
        void to_file(std::string fn);
        static ResultsPtr from_file(std::string fn);

        virtual ResultType & __add__(const ResultType &);
        virtual ResultType & __mul__(const double &);

        virtual void print(std::ostream &) const;
        virtual void print() const { print(std::cout); };

        virtual std::vector<std::string> h1_names() const {return list<H1>();};
        virtual std::vector<std::string> h2_names() const {return list<H2>();};;
        virtual std::vector<std::string> cf_names() const {return list<Cutflow>();};;

        // Get Histograms or Cutflows
        H1Ptr h1(string name);
        H1Ptr h1(string name, const uint32_t &bins, const double &min, const double &max);
        H2Ptr h2(string name);
        H2Ptr h2(string name, const uint32_t &xbins, const double &xmin, const double &xmax, const uint32_t &ybins, const double &ymin, const double &ymax);
        CutflowPtr cf(string name);

    private:
        string title;
};
typedef boost::shared_ptr<Results> ResultsPtr;

#endif
