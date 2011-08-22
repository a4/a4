#include <fstream>

#include <boost/format.hpp>

#include <stdexcept>

#include "a4/h1.h"
#include "a4/processor.h"
#include "a4/results.h"
#include "a4/reader.h"

#include "pb/Event.pb.h"



Processor::Processor():
    _current_file(""),
    _events_read(0),
    _events_read_in_last_file(0)
{
    _results.reset(new Results());
}

void Processor::init_output(const fs::path &outfile) {
    _writer.reset(new Writer(outfile, "Event", Event::kCLASSIDFieldNumber));
}

void Processor::event_passed(Event & e) {
    if (_writer) _writer->write(e);
};

int Processor::_histogram_fast_access_id = 0;

void Processor::process_work_unit(string fn)
{
    _current_file = fn;

    fs::path file(fn);

    Reader reader(file);
    _events_read_in_last_file = 0;

    // Process Events
    Event event;
    while(reader.good() && reader.read_event(event)) {
        process_event(event);
    }

    _events_read_in_last_file = reader.eventsRead();
    _events_read += _events_read_in_last_file;
}

ProcessingJob::ProcessingJob(): 
    _num_processors(0) 
{
    _results.reset(new Results());
};

ProcessorPtr ProcessingJob::get_configured_processor() {
    ProcessorPtr p = get_processor();
    _num_processors++;
    if (_output.size() != 0) {
        if (_threaded) {
            string fn = (boost::format("%1%_%2%.tmp") % _output % _num_processors).str();
            fs::path out_file_path(fn);
            p->init_output(out_file_path);
            _output_files.push_back(fn);
        } else if (_num_processors == 1) { 
            fs::path out_file_path(_output);
            p->init_output(out_file_path);
        } else {
            throw std::runtime_error("More than one processor without threads? Madness!");
        }
    }
    return p;
}

void ProcessingJob::worker_finished(WorkerPtr w) {
    Processor * p = static_cast<Processor *>(w.get());
    *_results += *p->results();
};

using namespace std;
void ProcessingJob::finalize() {
    if (_output_files.size()) {
        // concatenate all output files
        std::fstream out(_output.c_str(), ios::out | ios::trunc | ios::binary);
        for (vector<string>::const_iterator fn = _output_files.begin();
             fn != _output_files.end();
             fn++) {
            std::fstream in((*fn).c_str(), ios::in | ios::binary);
            out << in.rdbuf();
            fs::remove(fs::path(*fn));
        }
    }
}
