#ifndef _INSTRUCTOR_H
#define _INSTRUCTOR_H

#include <list>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <a4/worker.h>

namespace boost {
    class mutex;
    class condition_variable;
    class thread;
}

class Instructor
{
    public:
        Instructor(WorkerAgency *, const uint32_t &max_threads = 0);
        void submit_work_units(const std::vector<Worker::WorkUnit> &);

        void loop_until_done() const;
        bool is_done() const;

    private:
        typedef boost::shared_ptr<Worker::WorkUnit> WorkUnitPtr;

        class Thread {
            public:
                Thread(Instructor &, const int &id, WorkerPtr);
                virtual ~Thread() {};
                virtual void join() {_thread.join();};
                WorkerPtr worker() const {return _worker;};
                int id() const {return _thread_id;};
            private:
                virtual void loop();

                Instructor & _instructor;
                WorkerPtr _worker;

                int _thread_id;
                boost::thread _thread;
        };

        void launch_threads();
        void cleanup() const;

        WorkUnitPtr thread_instruct(Thread *thread);

        // Todo Items
        std::list<Worker::WorkUnit> _todo;

        // Thread pool information
        int _max_threads;
        int _max_thread_id;

        typedef boost::shared_ptr<Thread> ThreadPtr;
        std::map<int,ThreadPtr> _threads;
        mutable std::vector<ThreadPtr> _finished_threads;

        // Instructor Mutex
        mutable boost::mutex _mutex;
        mutable boost::condition_variable _waiting;

        WorkerAgency * _worker_agency;
};

#endif
