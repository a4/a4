#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "a4/processor.h"
#include "a4/results.h"

class Example : public Processor
{
    public:
        virtual void process_event(Event & event);
};


class ExampleJob : public ProcessingJob
{
    public:
        virtual ProcessorPtr get_processor() { return ProcessorPtr(new Example()); };
};

#endif
