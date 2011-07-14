#ifndef A4_PROCESSOR_H
#define A4_PROCESSOR_H

#include <vector>
#include <string>

using std::vector;
using std::string;

#include <boost/shared_ptr.hpp>

#include "a4/worker.h"
#include "a4/results.h"
#include "a4/reader.h"
#include "a4/writer.h"

class Event;

/*
 * A Processor is a specialized Worker.
 * It reads Events from A4 files, and processes them one by one.
 * The results are kept by default in a "Results" object, which is 
 * merged by the ProcessingJob at the end of processing.
 *
 * Override Processor::process_event and put your analysis there.
 * You also have to override ProcessingJob::get_processor() so
 * that it returns your new object!
 * There you can also initialize tools and hand them to your 
 * Analysis Processor
 */
class Processor : public Worker
{
    public:
        Processor();
        virtual ~Processor() {}

        // Override this for your Analysis
        virtual void process_event(Event &event) {};

        // You should not need to touch the rest...
        void init_output(const string &outfile);
        void event_passed(Event &);

        virtual uint32_t eventsRead() const {return _events_read;};
        virtual uint32_t eventsReadInLastFile() const {return _events_read_in_last_file;};

        virtual void process_work_unit(string fn);
        virtual ResultsPtr results() const {return _results;};

    protected:
        void write_event(Event &event) {if(_writer) _writer->write(event);};
        boost::shared_ptr<Results> _results;
        string _current_file;

        static int _histogram_fast_access_id ;
        vector<H1Ptr> _histogram_fast_access;
        inline H1Ptr _hfast(int id, const char * name, int nbin, double xmin, double xmax) {
            try {
                H1Ptr & h = _histogram_fast_access.at(id);
                if (h.get() != NULL) {
                    return h;
                }
            } catch (std::out_of_range & oor) {
                _histogram_fast_access.resize(id+1);
            }
            H1Ptr h = _results->h1(name, nbin, xmin, xmax);
            _histogram_fast_access[id] = h;
            return h;
        }

    private:
        boost::shared_ptr<Writer> _writer;
        uint32_t _events_read;
        uint32_t _events_read_in_last_file;
};
typedef boost::shared_ptr<Processor> ProcessorPtr;

class ProcessingJob : public WorkerAgency
{
    public:
        typedef vector<string> Inputs;

        ProcessingJob();
        virtual ~ProcessingJob() {};

        virtual WorkerPtr get_configured_worker() {return get_configured_processor();};
        virtual ProcessorPtr get_configured_processor();

        virtual void worker_finished(WorkerPtr);

        bool process_files(Inputs inputs);

        virtual ProcessorPtr get_processor() = 0;

        virtual ResultsPtr results() const {return _results;};
        virtual void set_output(string output) {_output = output;};

        virtual void finalize();

    private:
        int _num_processors;
        Inputs _inputs;
        Inputs _output_files;
        string _output;
        ResultsPtr _results;

};

typedef boost::shared_ptr<ProcessingJob> ProcessingJobPtr;

#endif
