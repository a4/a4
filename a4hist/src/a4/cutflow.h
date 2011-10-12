#ifndef _A4_CUTFLOW_H_
#define _A4_CUTFLOW_H_

#include <string>

#include <a4/storable.h>
#include <a4/axis.h>

#include <Cutflow.pb.h>

namespace a4{ namespace hist{

class Cutflow : public a4::process::StorableAs<Cutflow, pb::Cutflow>
{
    public:
        class CutNameCount {
            public: 
                std::string name; 
                double count, weights_squared; 
                CutNameCount(std::string n, double c, double w2) : name(n), count(c), weights_squared(w2) {};
        };

        Cutflow();
        Cutflow(const Cutflow &);
        ~Cutflow();

        // Implements StorableAs
        virtual void to_pb(bool blank_pb);
        virtual void from_pb();
        virtual Cutflow & operator+=(const Cutflow &other);

        std::vector<CutNameCount> content() const;

        Cutflow & __add__(const Cutflow &);
        Cutflow & __mul__(const double &);

        void print(std::ostream &) const;

        void fill(const int & id, const std::string & name, const double w = 1.0);

    private:
        std::vector<double> _fast_access_bin;
        shared<std::vector<double> > _weights_squared;
        std::vector<std::string> _cut_names;
};

};}; //namespace a4::hist

#endif

