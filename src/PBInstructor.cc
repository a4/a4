/**
 * ProtoBuf Thread(s) Instructor:
 *  - Create threads
 *  - synchronize
 *
 * Created by Samvel Khalatyan on Mar 10, 2011
 * Copyright 2011, All rights reserved
 */

#include <iostream>

#include <boost/filesystem.hpp>

#include "Condition.h"
#include "Cout.h"
#include "PBInstructor.h"
#include "PBThread.h"
#include "Results.h"

namespace fs = boost::filesystem;

using namespace std;

pb::Instructor::Instructor(const uint32_t &max_threads):
    _next_file(_input_files.begin()),
    _running_threads(0)
{
    _condition.reset(new Condition());

    _max_threads = max_threads
        ? max_threads
        : boost::thread::hardware_concurrency();

    _results.reset(new Results());

    _out.reset(new Cout());
}

void pb::Instructor::processFiles(const Files &files)
{
    // Do nothing if there are already running threads or there is nothing to do
    //
    {
        Lock lock(*_condition->mutex());
        if (files.empty() ||
            0 < _running_threads)

            return;

        _input_files = files;
        _next_file = _input_files.begin();

        _running_threads = 0;
    }

    // Start processing files
    //
    process();
}

// Communication with Thread
//
::Thread::ConditionPtr pb::Instructor::condition() const
{
    return _condition;
}

void pb::Instructor::notify(Thread *thread)
{
    Lock lock(*_condition->mutex());

    _complete_threads.push(thread);
}

pb::Instructor::ResultsPtr pb::Instructor::results() const
{
    return _results;
}

pb::Instructor::CoutPtr pb::Instructor::out() const
{
    return _out;
}



// Private
//
void pb::Instructor::process()
{
    // Create Threads
    //
    init();

    // Start Threads
    //
    start();

    // Run Threads
    //
    loop();
}

void pb::Instructor::init()
{
    Lock lock(*_condition->mutex());

    const int input_files = _input_files.size();

    for(int threads = (_max_threads
                        ? (_max_threads < input_files
                            ? _max_threads
                            : input_files)
                        : 1);
        0 < threads;
        --threads)
    {
        ThreadPtr thread(new Thread(this));

        thread->setId(threads);

        _threads.push_back(thread);
    }
}

void pb::Instructor::start()
{
    Lock lock(*_condition->mutex());

    for(Threads::iterator thread_iter = _threads.begin();
        _threads.end() != thread_iter;
        ++thread_iter)
    {
        Thread *thread = thread_iter->get();

        thread->init(fs::path(*_next_file));

        if (!thread->start())
            continue;

        ++_next_file;
        ++_running_threads;
    }
}

void pb::Instructor::loop()
{
    while(0 < _running_threads)
    {
        wait();

        processCompleteThreads();
    }
}

void pb::Instructor::wait()
{
    Lock lock(*_condition->mutex());

    // Wait for any thread to complete
    //
    while(_complete_threads.empty())
        _condition->variable()->wait(lock);
}

void pb::Instructor::processCompleteThreads()
{
    Lock lock(*_condition->mutex());
    while(!_complete_threads.empty())
    {
        if (_input_files.end() == _next_file)
            stopThread();
        else
            continueThread();
    }
}

void pb::Instructor::stopThread()
{
    Thread *thread = _complete_threads.front();
    _complete_threads.pop();

    thread->stop();
    thread->condition()->variable()->notify_all();

    // Potential delay. Threads can be added to the thread group instead
    // and then "joined" (or waited for all to finish)
    //
    thread->join();

    cout << "Thread read " << thread->eventsRead() << " events" << endl;

    _results->add(*thread->results());

    --_running_threads;
}

void pb::Instructor::continueThread()
{
    Thread *thread = _complete_threads.front();
    _complete_threads.pop();

    thread->init(*_next_file);
    ++_next_file;

    thread->condition()->variable()->notify_all();
}
