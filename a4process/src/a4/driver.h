#ifndef _A4_DRIVER_H_
#define _A4_DRIVER_H_

#include <a4/a4process.h>

namespace a4{ namespace process{

class Driver {
    protected:
        void set_instream(Processor* p, shared<a4::io::A4InputStream> instream) { p->_instream = instream; };
};

};}; // namespace a4::process

#endif
