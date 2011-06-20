/**
 * Process ProtoBuf file in thread: read events
 *
 * Created by Samvel Khalatyan on Mar 10, 2011
 * Copyright 2011, All rights reserved
 */

#include "thread.h"

#include "Cout.h"

#include "a4/processor.h"
#include "a4/results.h"

#include "instructor.h"



Thread::Thread(Instructor *instructor, ProcessorPtr processor):
    _wait_for_instructions(false),
    _continue(false),
    _processor(processor),
    _thread_id(0)
{
    _instructor = instructor;
    _out = _instructor->out();

    _condition.reset(new Condition());
}

bool Thread::start()
{
    // Start only if thread is not running so far
    //
    if (boost::thread() != _thread)
        return false;

    _thread = boost::thread(&Thread::loop, this);

    return true;
}

void Thread::stop()
{
    Lock lock(*_condition->mutex());

    _continue = false;
    _wait_for_instructions = false;
}

void Thread::join()
{
    _thread.join();
}

Thread::ConditionPtr Thread::condition() const
{
    return _condition;
}

void Thread::init(const fs::path &file)
{
    Lock lock(*_condition->mutex());

    _processor->init(file);
    _continue = true;
    _wait_for_instructions = false;

    _out->print(_thread_id, file.string());
}

uint32_t Thread::eventsRead() const
{
    Lock lock(*_condition->mutex());

    return _processor->eventsRead();
}

Thread::ResultsPtr Thread::results() const
{
    return _processor->results();
}

void Thread::setId(const int &id)
{
    _thread_id = id;
}



// Private
//
void Thread::loop()
{
    // thread will execute only in case input file was initally set. Otherwise
    // thread will quit
    //
    for(_wait_for_instructions = true;
        _continue;
        _wait_for_instructions = true)
    {
        // Process instructions
        //
        process();

        // Inform collector that Thread has finished its job and is waiting for
        // new instructions
        //
        notify();

        // Wait for new instructions
        //
        wait();
    }
}

void Thread::process()
{
    _processor->processEvents();
}

void Thread::notify()
{
    _instructor->notify(this);
}

void Thread::wait()
{
    Lock lock(*_condition->mutex());

    while(_wait_for_instructions)
    {
        _instructor->condition()->variable()->notify_all();
        _condition->variable()->wait(lock);
    }
}
