/**
 * Thread interface
 *
 * Created by Samvel Khalatyan on Mar 10, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef THREAD_H
#define THREAD_H

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

class Condition;

class Thread
{
    public:
        typedef boost::shared_ptr<Condition> ConditionPtr;
        typedef boost::unique_lock<boost::mutex> Lock;

        virtual ~Thread() {}

        virtual bool start() = 0;

        virtual void stop() = 0;

        virtual void join() = 0;

        virtual ConditionPtr condition() const = 0;
};

#endif
