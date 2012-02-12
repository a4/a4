#ifndef _A4_STRING_H_
#define _A4_STRING_H_

#include <sstream>

template<typename... Args>
static inline std::string str_printf(const char* s, const Args&... args);

template<typename... Args>
static inline std::string str_cat(const Args&... args);

#include <a4/string_impl.h>

#endif
