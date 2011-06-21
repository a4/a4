#include <fstream>

#include <boost/format.hpp>

#include "a4/H1.h"
#include "a4/processor.h"
#include "a4/results.h"
#include "a4/reader.h"

#include "pb/Event.pb.h"

#include "instructor.h"


Processor::Processor():
    _events_read(0),
    _events_read_in_last_file(0)
{
    _results.reset(new Results());
}

void Processor::init_output(const fs::path &outfile) {
    _writer.reset(new Writer(outfile, "Event", Event::kCLASSIDFieldNumber));
}

void Processor::init(const fs::path &file)
{
    _reader.reset(new Reader(file));
    _events_read_in_last_file = 0;
}

void Processor::process_stream()
{
    if (!_reader)
        return;

    process();

    _events_read_in_last_file = _reader->eventsRead();
    _events_read += _events_read_in_last_file;

    _reader.reset();
}

void Processor::process()
{
    Event event;

    while(_reader->good() && _reader->read_event(event)) {
        process_event(event);
    }
}

void Processor::close()
{
    if (_writer) {
        _writer.reset();
    }
}

ResultsPtr Processor::results() const
{
    return _results;
}

uint32_t Processor::eventsRead() const
{
    return _events_read;
}

uint32_t Processor::eventsReadInLastFile() const
{
    return _events_read_in_last_file;
}

ProcessingJob::ProcessingJob() : _threads(-1), _num_processors(0) {};

bool ProcessingJob::process_files(ProcessingJob::Inputs inputs) {
    if (_threads != -1)
    {
        boost::shared_ptr<Instructor> instructor(new Instructor(this, _threads));
        instructor->process_files(inputs);
        _results = instructor->results();
    }
    else
    {
        boost::shared_ptr<Processor> processor;
        processor = get_configured_processor();

        for(Inputs::const_iterator input = inputs.begin();
            inputs.end() != input;
            ++input)
        {
            fs::path file_path(*input);
            processor->init(file_path);
            processor->process_stream();
        }
        processor->close();
        _results = processor->results();
    }
    return true;
}

ProcessorPtr ProcessingJob::get_configured_processor() {
    ProcessorPtr p = get_processor();
    _num_processors++;
    if (_output.size() != 0) {
        if (_threads != -1) {
            string fn = (boost::format("%1%_%2%.tmp") % _output % _num_processors).str();
            fs::path out_file_path(fn);
            p->init_output(out_file_path);
            _output_files.push_back(fn);
        } else if (_num_processors == 1) { 
            fs::path out_file_path(_output);
            p->init_output(out_file_path);
        } else {
            throw "More than one processor without threads? Madness!";
        }
    }
    return p;
}

using namespace std;
void ProcessingJob::finalize() {
    if (_output_files.size()) {
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
