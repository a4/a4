/**
 * Processor Interface
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

namespace fs = boost::filesystem;

class Results;

class Processor
{
    public:
        typedef boost::shared_ptr<Results> ResultsPtr;

        virtual ~Processor() {}

        virtual void init(const fs::path &file) = 0;
        virtual void processEvents() = 0;

        virtual ResultsPtr results() const = 0;

        virtual uint32_t eventsRead() const = 0;
        virtual uint32_t eventsReadInLastFile() const = 0;
};

#endif
