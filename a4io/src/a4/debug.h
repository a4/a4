#ifndef _A4_DEBUG_H_
#define _A4_DEBUG_H_

#include <iostream>

#include <a4/string.h>
#include <a4/types.h>

namespace a4 { namespace io {

    /// Initialize the logging to use the real program name instead of "a4"
    void set_program_name(const char * program_name);

    /// Override log level given in A4_LOG_LEVEL with verbose/quiet from
    /// the command line. Log priorities are:
    /// * error = 1
    /// * warning = 2
    /// * notice = 3
    /// * verbose = 4
    /// * debug = 5
    /// Only messages with a priority less or equal to the log level are shown.
    void set_log_level(int level = 3);

    /// Extern booleans that are set by init_logging or using A4_LOG_LEVEL
    extern bool log_error;
    extern bool log_warning;
    extern bool log_notice;
    extern bool log_verbose;
    extern bool log_debug;

    /// Program name that prefixes the logging messages.
    extern const char * program_name;

};};

#define TERMINATE(...) \
    do { \
        throw a4::Terminate(__VA_ARGS__); \
    } while(0)
#define TERMINATE_ASSERT(A, ...) do { if(!(A)) { TERMINATE( __VA_ARGS__ ); } } while (0)

#define FATAL(...) \
    do { \
        throw a4::Fatal(__VA_ARGS__); \
    } while(0)
#define FATAL_ASSERT(A, ...) do { if(!(A)) { FATAL( __VA_ARGS__ ); } } while (0)

#define A4_LOG_IF(cond, level, levelstr, ...) \
    do {\
        if (a4::io::log_ ## level and cond) { \
            std::cerr << a4::io::program_name << ": " levelstr \
                << str_cat(  __VA_ARGS__  ) << std::endl;\
        }\
    } while (0)
    
#define ERROR(...)   A4_LOG_IF(true, error,   "Error: ", __VA_ARGS__ )
#define WARNING(...) A4_LOG_IF(true, warning, "Warning: ", __VA_ARGS__ )
#define INFO(...)    A4_LOG_IF(true, notice, "", __VA_ARGS__ )
#define VERBOSE(...) A4_LOG_IF(true, verbose, "(verbose): ", __VA_ARGS__ )

#define ERROR_ASSERT(A, ...)   A4_LOG_IF(A, error,   "Error: ", __VA_ARGS__ )
#define WARNING_ASSERT(A, ...) A4_LOG_IF(A, warning, "Warning: ", __VA_ARGS__ )
#define INFO_ASSERT(A, ...)    A4_LOG_IF(A, notice,  "", __VA_ARGS__ )
#define VERBOSE_ASSERT(A, ...) A4_LOG_IF(A, verbose, "(verbose): ", __VA_ARGS__ )

#ifndef NDEBUG
#define DEBUG(...)   A4_LOG_IF(true, debug,   "(debug): ", __VA_ARGS__ )
#define DEBUG_ASSERT(A, ...)   A4_LOG_IF(A, debug,   "(debug): ", __VA_ARGS__ )
#else
#define DEBUG(...)
#define DEBUG_ASSERT(A, ...)
#endif

// Used to silence
// "error: ignoring return value of ‘func’, declared with attribute warn_unused_result
// (but only when we know what we're doing!)
#define A4_UNUSED(expression) ((void)expression)

#endif // _A4_DEBUG_H

