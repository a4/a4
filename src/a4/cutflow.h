#ifndef _A4_CUTFLOW_H_
#define _A4_CUTFLOW_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

class Cutflow 
{
    public:
        class CutNameCount { public: std::string name; double count; CutNameCount(std::string n, double c) : name(n), count(c) {};};

        Cutflow();
        Cutflow(const Cutflow &);
        ~Cutflow();

        std::vector<CutNameCount> content() const;

        void add(const Cutflow &);
        void print(std::ostream &) const;

        void fill(const int & id, const std::string & name, const double w = 1.0);

        static int _fast_access_id;
    private:
        std::vector<double> _fast_access_bin;
        std::vector<std::string> _cut_names;
};
typedef boost::shared_ptr<Cutflow> CutflowPtr;

#endif

