#ifndef A4_PROCESSOR_H
#define A4_PROCESSOR_H

#include <vector>
#include <string>

using std::vector;
using std::string;

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

namespace fs = boost::filesystem;

#include "a4/results.h"
#include "a4/reader.h"

class Processor
{
    public:
        Processor();
        virtual ~Processor() {}

        virtual void init(const fs::path &file);
        virtual void processEvents();
        virtual void process();

        virtual ResultsPtr results() const;

        virtual uint32_t eventsRead() const;
        virtual uint32_t eventsReadInLastFile() const;

    protected:
        boost::shared_ptr<Reader> _reader;
        boost::shared_ptr<Results> _results;

    private:
        uint32_t _events_read;
        uint32_t _events_read_in_last_file;
};

typedef boost::shared_ptr<Processor> ProcessorPtr;

class ProcessingJob
{
    public:
        typedef vector<string> Inputs;

        ProcessingJob();
        virtual ~ProcessingJob() {}

        virtual ProcessorPtr get_processor() = 0;

        virtual ResultsPtr results() const {return _results;};
        virtual void set_threads(int threads) {_threads = threads;};
        virtual bool process_files(vector<string>);
    private:
        int _threads;
        ResultsPtr _results;
};

typedef boost::shared_ptr<ProcessingJob> ProcessingJobPtr;

#endif
