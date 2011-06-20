/**
 * cout thread-safe wrapper
 *
 * Created by Samvel Khalatyan on Mar 14, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef COUT_H
#define COUT_H

#include <boost/shared_ptr.hpp>

#include <string>

class Condition;

class Cout
{
    public:
        Cout();

        void print(const std::string &);
        void print(const int &, const std::string &);

    private:
        typedef boost::shared_ptr<Condition> ConditionPtr;

        ConditionPtr _condition;
};

#endif
