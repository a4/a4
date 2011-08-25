#ifndef _A4_STREAMABLE_H
#define _A4_STREAMABLE_H

#include <a4/writer.h>
#include <boost/shared_ptr.hpp>


class streamable {
    public:
        virtual MessagePtr get_message() = 0;
};

#endif
