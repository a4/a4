/**
 * Thread Condition pair of variables and mutex
 *
 * Created by Samvel Khalatyan on Mar 10, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef CONDITION_H
#define CONDITION_H

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

class Condition
{
    public:
        typedef boost::shared_ptr<boost::mutex> MutexPtr;
        typedef boost::shared_ptr<boost::condition_variable> VariablePtr;

        Condition() throw();

        MutexPtr mutex() const;
        VariablePtr variable() const;

    private:
        MutexPtr    _mutex;
        VariablePtr _variable;
};

#endif
