/**
 * Condition
 * container with mutex and condition variable
 *
 * Created by Samvel Khalatian on Feb 22, 2011
 * Copyright 2011, All rights reserved
 */

#include <iostream>

#include "Condition.h"

using std::cerr;
using std::endl;

Condition::Condition() throw()
try
{
    _mutex.reset(new boost::mutex());
    _variable.reset(new boost::condition_variable());
}
catch(const std::bad_alloc &)
{
    cerr << "condition: Failed to allocate memory" << endl;
}

Condition::MutexPtr Condition::mutex() const
{
    return _mutex;
}

Condition::VariablePtr Condition::variable() const
{
    return _variable;
}
