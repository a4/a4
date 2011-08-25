#ifndef _A4_CUTFLOW_H_
#define _A4_CUTFLOW_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <a4/streamable.h>
#include <a4/result_type.h>

class Cutflow;
typedef boost::shared_ptr<Cutflow> CutflowPtr;

class Cutflow : public ResultType
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
        Cutflow(Message &); 
        ~Cutflow();

        virtual void from_message(google::protobuf::Message & m);
        virtual MessagePtr get_message();

        std::vector<CutNameCount> content() const;

        ResultType & __add__(const ResultType &);
        ResultType & __mul__(const double &);

        void print(std::ostream &) const;

        void fill(const int & id, const std::string & name, const double w = 1.0);

        static int _fast_access_id;



    private:
        std::vector<double> _fast_access_bin;
        boost::shared_ptr<std::vector<double> > _weights_squared;
        std::vector<std::string> _cut_names;
};


#endif

