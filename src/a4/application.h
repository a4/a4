#ifndef A4_APPLICATION_H
#define A4_APPLICATION_H

#include "a4/processor.h"

#define FACTORY(PROCESSOR) class MyProcessingJob : public ProcessingJob { public: virtual ProcessorPtr get_processor() { return ProcessorPtr(new PROCESSOR()); }; };

#define A4_MAIN(PROCESSOR) FACTORY(PROCESSOR) \
                           int main(int argc, char ** argv) { ProcessingJobPtr pf(new MyProcessingJob()); return a4_main(argc, argv, pf); };

int a4_main(int argc, char *argv[], ProcessingJobPtr pf);

#endif
