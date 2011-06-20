/**
 * Process ProtoBuf file in thread: read events
 *
 * Created by Samvel Khalatyan on Mar 10, 2011
 * Copyright 2011, All rights reserved
 */

#include "Cout.h"
#include "PBInstructor.h"
#include "PBProcessor.h"
#include "PBThread.h"
#include "Results.h"

pb::Thread::Thread(Instructor *instructor):
    _wait_for_instructions(false),
    _continue(false),
    _thread_id(0)
{
    _instructor = instructor;
    _out = _instructor->out();

    _condition.reset(new Condition());
    _processor.reset(new Processor());
}

bool pb::Thread::start()
{
    // Start only if thread is not running so far
    //
    if (boost::thread() != _thread)
        return false;

    _thread = boost::thread(&Thread::loop, this);

    return true;
}

void pb::Thread::stop()
{
    Lock lock(*_condition->mutex());

    _continue = false;
    _wait_for_instructions = false;
}

void pb::Thread::join()
{
    _thread.join();
}

::Thread::ConditionPtr pb::Thread::condition() const
{
    return _condition;
}

void pb::Thread::init(const fs::path &file)
{
    Lock lock(*_condition->mutex());

    _processor->init(file);
    _continue = true;
    _wait_for_instructions = false;

    _out->print(_thread_id, file.string());
}

uint32_t pb::Thread::eventsRead() const
{
    Lock lock(*_condition->mutex());

    return _processor->eventsRead();
}

pb::Thread::ResultsPtr pb::Thread::results() const
{
    return _processor->results();
}

void pb::Thread::setId(const int &id)
{
    _thread_id = id;
}



// Private
//
void pb::Thread::loop()
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

void pb::Thread::process()
{
    _processor->processEvents();
}

void pb::Thread::notify()
{
    _instructor->notify(this);
}

void pb::Thread::wait()
{
    Lock lock(*_condition->mutex());

    while(_wait_for_instructions)
    {
        _instructor->condition()->variable()->notify_all();
        _condition->variable()->wait(lock);
    }
}
