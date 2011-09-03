#ifndef A4_PROCESSOR_H
#define A4_PROCESSOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include "a4/worker.h"
#include "a4/results.h"
#include "a4/reader.h"
#include "a4/writer.h"

class Event;
template <class MyProcessor> class JobConfiguration;

/*
 * A Processor is a specialized Worker.
 * It reads events from A4 files, and processes them one by one.
 * The results are kept by default in a "Results" object, which is 
 * merged by the JobConfiguration at the end of processing.
 *
 * Override Processor::process_event and put your analysis there.
 */
class Processor : public Worker
{
    public:
        Processor();
        virtual ~Processor() {}

        // Override this for your Analysis
        virtual void process_event(Event &event) {};

        // You should not need to touch the rest...
        void init_output(const std::string &outfile);
        void event_passed(Event &);

        virtual uint32_t eventsRead() const {return _events_read;};
        virtual uint32_t eventsReadInLastFile() const {return _events_read_in_last_file;};

        virtual void process_work_unit(std::string fn);
        virtual ResultsPtr results() const {return _results;};

    protected:
        void write_event(Event &event) {if(_writer) _writer->write(event);};
        ResultsPtr _results;
        std::string _current_file;

        static int _histogram_fast_access_id ;
        std::vector<H1Ptr> _histogram_fast_access;
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
        JobConfiguration * const config;
        boost::shared_ptr<Writer> _writer;
        uint32_t _events_read;
        uint32_t _events_read_in_last_file;

        friend class JobConfiguration;
};
typedef boost::shared_ptr<Processor> ProcessorPtr;

template <MyProcessor>
class JobConfiguration : public WorkerAgency
{
    public:
        typedef std::vector<std::string> FileList;
        typedef boost::program_options::variables_map ConfigArguments;
        using boost::program_options::value;

        JobConfiguration();
        virtual ~JobConfiguration() {};

        
        // Override these three for your analysis
        virtual boost::program_options get_options();
        virtual bool initialize(ConfigArguments &);
        virtual bool configure(MyProcessor &);

        virtual WorkerPtr get_configured_worker() {return get_configured_processor();};

        virtual ProcessorPtr get_configured_processor() {
            MyProcessor * p = new MyProcessor();
            p->set_shared(this);
            assert(configure_processor(*p));
            return ProcessorPtr(p)
        };

        virtual void worker_finished(WorkerPtr);

        bool process_files(FileList inputs);

        virtual ResultsPtr results() const {return _results;};
        virtual void set_output(std::string output) {_output = output;};

        virtual void finalize();

    private:
        int _num_processors;
        FileList _inputs;
        FileList _output_files;
        std::string _output;
        ResultsPtr _results;

};

typedef boost::shared_ptr<JobConfiguration> JobConfigurationPtr;

#endif
