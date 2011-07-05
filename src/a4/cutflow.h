#ifndef _A4_CUTFLOW_H_
#define _A4_CUTFLOW_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <a4/streamable.h>

class Cutflow;
typedef boost::shared_ptr<Cutflow> CutflowPtr;

class Cutflow : public streamable
{
    public:
        class CutNameCount { public: std::string name; double count; CutNameCount(std::string n, double c) : name(n), count(c) {};};

        Cutflow();
        Cutflow(const Cutflow &);
        Cutflow(Message &); 
        ~Cutflow();

        std::vector<CutNameCount> content() const;

        void add(const Cutflow &);
        void print(std::ostream &) const;

        void fill(const int & id, const std::string & name, const double w = 1.0);

        static int _fast_access_id;

        virtual MessagePtr get_message();

        Cutflow & operator*=(const double &);
    private:
        std::vector<double> _fast_access_bin;
        std::vector<std::string> _cut_names;
};


#endif

