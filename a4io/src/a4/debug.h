#ifndef __A4_DEBUG_H
#define __A4_DEBUG_H

#include <iostream>

#include <a4/string.h>

#define ERROR(...)   std::cerr << "[ERROR]   (" << __FILE__ << ":" << __LINE__ << ") " << str_cat(  __VA_ARGS__  ) << std::endl;
#define WARNING(...) std::cerr << "[WARNING] (" << __FILE__ << ":" << __LINE__ << ") " << str_cat(  __VA_ARGS__  ) << std::endl;
#define INFO(...)    std::cerr << "[INFO]    (" << __FILE__ << ":" << __LINE__ << ") " << str_cat(  __VA_ARGS__  ) << std::endl;
#define VERBOSE(...) std::cerr << "[VERBOSE] (" << __FILE__ << ":" << __LINE__ << ") " << str_cat(  __VA_ARGS__  ) << std::endl;
#ifdef NDEBUG
#define DEBUG(...)
#else
#define DEBUG(...)   std::cerr << "[DEBUG]   (" << __FILE__ << ":" << __LINE__ << ") " << str_cat(  __VA_ARGS__  ) << std::endl;
#endif

#ifdef NDEBUG
#define DEBUG_ASSERT(A, ...)
#else
#define DEBUG_ASSERT(A, ...) if(!(A)) { DEBUG( __VA_ARGS__ ); }
#endif
#define ERROR_ASSERT(A, ...) if(!(A)) { ERROR( ##__VA_ARGS__ ); }
#define WARNING_ASSERT(A, ...) if(!(A)) { WARNING( ##__VA_ARGS__ ); }
#define INFO_ASSERT(A, ...) if(!(A)) { INFO( ##__VA_ARGS__ ); }
#define VERBOSE_ASSERT(A, ...) if(!(A)) { VERBOSE( ##__VA_ARGS__ ); }

#endif // __A4_DEBUG_H

