#ifndef A4_PROCESSOR_H
#define A4_PROCESSOR_H

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

namespace fs = boost::filesystem;

#include "a4/results.h"
#include "a4/reader.h"

class Processor
{
    public:
        typedef boost::shared_ptr<Results> ResultsPtr;

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

class ProcessorFactory
{
    public:
        virtual ProcessorPtr get_processor() = 0;
};

typedef boost::shared_ptr<ProcessorFactory> ProcessorFactoryPtr;

#endif
