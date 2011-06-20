/**
 * Process ProtoBuf file: read events
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef PROTOBUF_PROCESSOR_H
#define PROTOBUF_PROCESSOR_H

#include <boost/shared_ptr.hpp>

#include "Processor.h"

class H1;
class Results;

namespace pb
{
    class Reader;


    class Processor : public ::Processor
    {
        public:
            typedef ::Processor::ResultsPtr ResultsPtr;

            Processor();

            virtual void init(const fs::path &file);
            virtual void processEvents();

            virtual void process();

            virtual ResultsPtr results() const;

            virtual uint32_t eventsRead() const;
            virtual uint32_t eventsReadInLastFile() const;

        private:
            boost::shared_ptr<Reader> _reader;

            uint32_t _events_read;
            uint32_t _events_read_in_last_file;

            boost::shared_ptr<Results> _results;
    };
}

#endif
