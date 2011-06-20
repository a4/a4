/**
 * Process ProtoBuf file in thread: read events
 *
 * Created by Samvel Khalatyan on Mar 10, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef THREAD_H
#define THREAD_H

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "Condition.h"

#include "a4/processor.h"

namespace fs = boost::filesystem;

class Cout;
class Results;
class Instructor;

class Thread
{
    public:
        typedef boost::shared_ptr<Condition> ConditionPtr;
        typedef boost::unique_lock<boost::mutex> Lock;
        typedef boost::shared_ptr<Results> ResultsPtr;
        typedef boost::shared_ptr<Cout> CoutPtr;

        Thread(Instructor *instructor, ProcessorPtr);
        virtual ~Thread() {}

        // Thread interface
        //
        virtual bool start();
        virtual void stop();
        virtual void join();
        virtual ConditionPtr condition() const;

        // Send instructions to thread
        //
        void init(const fs::path &file);

        uint32_t eventsRead() const;

        ResultsPtr results() const;

        void setId(const int &);

    private:
        // Processing loop
        //
        void loop();

        // Process file
        //
        void process();

        // Notify instructor
        //
        void notify();

        // Wait for instructions
        //
        void wait();

        boost::thread _thread;
        ConditionPtr _condition;

        Instructor *_instructor;
        bool _wait_for_instructions;
        bool _continue;

        ProcessorPtr _processor;

        CoutPtr _out;

        int _thread_id;
};

#endif
