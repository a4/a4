#ifndef _WORKER_H_
#define _WORKER_H_

#include <string>
#include <vector>

namespace boost{template<class> class shared_ptr;}

/*
 * Note to Developers:
 * These interfaces exist to make it easy to write thread-safe code,
 * and are low-level.
 */

/*
 * "Worker" is an interface.
 * An "Instructor" gives orders to a worker, via "process_work_unit".
 * A good worker should keep to itself and not interfere with other workers
 * (process_work_unit will only be executed in one thread on one object,
 * but beware of static methods and static variables in member functions!)
 * A good worker should also expect more than one work unit,
 * and be ready to hand over the results of his work to his agency (see below)
 */
class Worker {
    public:
        typedef std::string WorkUnit;
        typedef std::vector<std::string> WorkUnits;
        virtual void process_work_unit(WorkUnit) = 0;
};
typedef boost::shared_ptr<Worker> WorkerPtr;

/*
 * "WorkerAgency" is an interface.
 * This Agency should be able to provide skilled Workers on request.
 * It should also get back and collate the results of the work being done,
 * as soon as a worker has finished.
 * process_work_units processes the given WorkUnits in threads
 * threads=-1 means "threads off", threads=0 means "# of cores"
 */
class WorkerAgency {
    public:
        virtual WorkerPtr get_configured_worker() = 0;
        virtual void worker_finished(WorkerPtr) = 0;
        void process_work_units(const Worker::WorkUnits&, const int &threads=0);
    protected:
        bool _threaded;
};

#endif
