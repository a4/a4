/**
 * Process ProtoBuf file in thread: read events
 *
 * Created by Samvel Khalatyan on Mar 10, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef PROTOBUF_THREAD_H
#define PROTOBUF_THREAD_H

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "Condition.h"
#include "Thread.h"

namespace fs = boost::filesystem;

class Cout;
class Results;

namespace pb
{
    class Instructor;
    class Processor;

    class Thread : public ::Thread
    {
        public:
            typedef boost::shared_ptr<Results> ResultsPtr;
            typedef boost::shared_ptr<Cout> CoutPtr;

            Thread(Instructor *instructor);

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

            boost::shared_ptr<Processor> _processor;

            CoutPtr _out;

            int _thread_id;
    };
}

#endif
