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

using namespace std;

class Instructor
{
    public:
        Instructor(WorkerAgency *, const uint32_t &max_threads = 0);
        void submit_work_units(const vector<Worker::WorkUnit> &);

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

                Instructor& _instructor;
                WorkerPtr _worker;

                int _thread_id;
                boost::thread _thread;
        };

        void launch_threads();
        void cleanup() const;

        WorkUnitPtr thread_instruct(Thread *thread);

        // Todo Items
        list<Worker::WorkUnit> _todo;

        // Thread pool information
        int _max_threads;
        int _max_thread_id;

        typedef boost::shared_ptr<Thread> ThreadPtr;
        map<int,ThreadPtr> _threads;
        mutable vector<ThreadPtr> _finished_threads;

        // Instructor Mutex
        mutable boost::mutex _mutex;
        mutable boost::condition_variable _waiting;

        WorkerAgency * _worker_agency;
};

void WorkerAgency::process_work_units(const Worker::WorkUnits &work_units, const int &threads) {
    Instructor instructor(this, threads);
    instructor.submit_work_units(work_units);
    instructor.loop_until_done();
}


typedef boost::unique_lock<boost::mutex> Lock;

using namespace std;

Instructor::Instructor(WorkerAgency * a, const uint32_t &max_threads):
    _max_threads(max_threads ? max_threads : boost::thread::hardware_concurrency()),
    _max_thread_id(0),
    _worker_agency(a)
{};

void Instructor::submit_work_units(const vector<Worker::WorkUnit> &wu)
{
    {
        Lock lock(_mutex);
        // Append the work units to the todo list 
        _todo.insert(_todo.end(), wu.begin(), wu.end());
    }
    // Up the number of running threads
    launch_threads();
};

// Create as many threads as are needed, up to _max_threads.
// Can be called multiple times if more input is added
void Instructor::launch_threads()
{
    Lock lock(_mutex);
    const int n_todo = _todo.size();
    const int n_threads = (_max_threads == 0 ? 1 : 
        ((_max_threads < n_todo) ? _max_threads : n_todo));

    for(int threads = _threads.size(); threads < n_threads; threads++) {
        WorkerPtr worker = _worker_agency->get_configured_worker();
        int id = ++_max_thread_id;
        _threads[id].reset(new Thread(*this, id, worker));
    }
}

Instructor::WorkUnitPtr Instructor::thread_instruct(Thread *thread)
{
    Lock lock(_mutex);
    if (_todo.size() > 0) {
        Instructor::WorkUnitPtr work_unit(new Worker::WorkUnit(_todo.front()));
        _todo.pop_front();
        return work_unit;
    }

    // Merge thread result
    _worker_agency->worker_finished(thread->worker());
    _finished_threads.push_back(_threads[thread->id()]);
    _threads.erase(thread->id());
    _waiting.notify_all();
    return Instructor::WorkUnitPtr();
}

void Instructor::loop_until_done() const
{
    Lock lock(_mutex);
    // Wait for any thread to complete, as long as any are running
    while(_threads.size() > 0) {
        _waiting.wait(lock);
        cleanup();
    }
    cleanup();
}

bool Instructor::is_done() const
{
    Lock lock(_mutex);
    cleanup();
    return (_threads.size() == 0);
}

void Instructor::cleanup() const {
    // Join all finished threads
    while(_finished_threads.size()) {
        ThreadPtr thread = _finished_threads.back();
        thread->join();
        _finished_threads.pop_back();
    };
}

Instructor::Thread::Thread(Instructor &instructor, const int &id, WorkerPtr worker):
    _instructor(instructor),
    _worker(worker),
    _thread_id(id),
    _thread(boost::thread(&Thread::loop, this))
{};

// Private
void Instructor::Thread::loop()
{
    boost::shared_ptr<Worker::WorkUnit> work_unit;
    while (work_unit = _instructor.thread_instruct(this)) {
        _worker->process_work_unit(*work_unit);
    }
}
