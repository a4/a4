#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "a4/processor.h"
#include "a4/results.h"

class CopyProcessor : public Processor
{
    public:
        virtual void process();
};


class CopyProcessorFactory : public ProcessorFactory
{
    public:
        virtual ProcessorPtr get_processor() { return ProcessorPtr(new CopyProcessor()); };
};

#endif
