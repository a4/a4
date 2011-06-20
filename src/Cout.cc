/**
 * cout thread-safe wrapper
 *
 * Created by Samvel Khalatyan on Mar 14, 2011
 * Copyright 2011, All rights reserved
 */

#include <iostream>

#include <boost/thread.hpp>

#include "Condition.h"
#include "Cout.h"

using namespace std;

Cout::Cout()
{
    _condition.reset(new Condition());
}

void Cout::print(const string &value)
{
    typedef boost::unique_lock<boost::mutex> Lock;

    Lock lock(*_condition->mutex());

    cout << boost::this_thread::get_id() << ": " << value << endl;;
}

void Cout::print(const int &id, const string &value)
{
    typedef boost::unique_lock<boost::mutex> Lock;

    Lock lock(*_condition->mutex());

    cout << id << ": " << value << endl;;
}
