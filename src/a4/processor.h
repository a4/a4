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
#include "a4/writer.h"

class Processor
{
    public:
        Processor();
        virtual ~Processor() {}

        virtual void init(const fs::path &file);
        virtual void init_output(const fs::path &file);

        virtual void process();

        virtual void process_stream();
        virtual ResultsPtr results() const;

        virtual uint32_t eventsRead() const;
        virtual uint32_t eventsReadInLastFile() const;

        virtual void close();
    protected:
        boost::shared_ptr<Reader> _reader;
        boost::shared_ptr<Writer> _writer;
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

        bool process_files(Inputs inputs);

        virtual ProcessorPtr get_processor() = 0;

        virtual ResultsPtr results() const {return _results;};
        virtual void set_threads(int threads) {_threads = threads;};
        virtual void set_output(string output) {_output = output;};

        virtual ProcessorPtr get_configured_processor();

        virtual void finalize();

    private:
        int _threads;
        int _num_processors;
        Inputs _inputs;
        Inputs _output_files;
        string _output;
        ResultsPtr _results;

};

typedef boost::shared_ptr<ProcessingJob> ProcessingJobPtr;

#endif
