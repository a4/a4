/// Runtime startup linkage check

/// Because we aren't able to guarantee ABI compatibility at the moment, we use
/// a heuristic to detect potentially incompatible libraries, and abort before
/// strange crashes or data corruption occurs.

/// The strategy is to use `dl_iterate_phdr` to determine which libraries are
/// loaded, look at their modification time. If any dynamic libraries are
/// younger than the current executable (as determined by dereferencing
/// /proc/self/exe), then it is possible that they may be incompatible and
/// LinkageCheck will bail out.

/// You may want to override this if you know that recent changes to the
/// any dynamic libraries are ABI-safe, which can be done by defining
/// A4_SKIP_LINKAGECHECK in your environment, or using `/bin/touch` on your
/// executable to make it younger.


#include <a4/debug.h>

#ifdef __linux__
# include "linkage_check_linux.h"
#else
# include "linkage_check_darwin.h"
#endif /* !__linux__ */

LinkageCheck linkage_check;
