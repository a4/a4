#ifndef _A4_DRIVER_H_
#define _A4_DRIVER_H_

#include <a4/a4process.h>

namespace a4{ namespace process{

class Driver {
    public:
        virtual a4::io::A4Message & metadata() = 0;
};

};}; // namespace a4::process

#endif
