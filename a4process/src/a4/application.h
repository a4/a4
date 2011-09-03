#ifndef A4_APPLICATION_H
#define A4_APPLICATION_H

#include "a4/processor.h"

#define a4_application(MyConfiguration) int main(int argc, char *argv[]) { return a4_main(argc, argv, new MyConfiguration()); };

int a4_main(int argc, char *argv[], JobConfiguration *);

#endif
