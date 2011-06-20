#ifndef A4_APPLICATION_H
#define A4_APPLICATION_H

#include <boost/shared_ptr.hpp>
#include "a4/processor.h"

int a4_main(int argc, char *argv[], ProcessorFactoryPtr pf, ResultsPtr &r);

#endif
