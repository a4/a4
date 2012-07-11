/// Runtime startup linkage check

#ifndef A4IO_LINKAGE_CHECK_DARWIN_H
#define A4IO_LINKAGE_CHECK_DARWIN_H 1

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class LinkageCheck {
public:
    LinkageCheck() {
        
        if (getenv("A4_SKIP_LINKAGECHECK") != NULL)
            return;
    }
};

#endif /* !A4IO_LINKAGE_CHECK_DARWIN_H */
