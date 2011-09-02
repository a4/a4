
#include "instructor.h"
#include "a4/worker.h"

void WorkerAgency::process_work_units(const Worker::WorkUnits &work_units, const int &threads) {
    if (threads != -1)
    {
        _threaded = true;
        Instructor instructor(this, threads);
        instructor.submit_work_units(work_units);
        instructor.loop_until_done();
    }
    else
    {
        _threaded = false;
        WorkerPtr worker = get_configured_worker();

        for(Worker::WorkUnits::const_iterator w = work_units.begin();
            w != work_units.end();
            w++)
        {
            worker->process_work_unit(*w);
        }
        worker_finished(worker);
    }
}
