#ifndef _A4_STRING_IMPL_H_
#define _A4_STRING_IMPL_H_

#include <a4/string.h>
#include <typeinfo>

namespace _string_internal {
    std::ostream& operator<<(std::ostream &sstr, const std::type_info& ti);

    static inline std::string str_printf(const char* s) {
        return std::string (s);
    }

    template<typename T, typename... Args>
    std::string str_printf(const char* s, const T& value, const Args&... args) {
        std::string res;
        while (*s) {
            if (*s == '%' && *++s != '%') {
                res += std::string(value);
                return res + str_printf(++s, args...);
            }
            res += char(*s++);
        }
        // Append extra arguments
        return res + std::string(value) + str_printf("", args...);
    }

    inline static void _stream_in(std::stringstream& ss) {};

    template<typename T, typename... Args>
    void _stream_in(std::stringstream& ss, const T& s, const Args&... args) {
        ss << s;
        _stream_in(ss, args...);
    }

    template<typename... Args>
    std::string str_cat(const Args&... args) {
        std::stringstream ss;
        _stream_in(ss, args...);
        return ss.str();
    }

};

template<typename... Args>
static inline std::string str_printf(const char* s, const Args&... args) {
    return _string_internal::str_printf(s, args...);
}

template<typename... Args>
static inline std::string str_cat(const Args&... args) {
    return _string_internal::str_cat(args...);
}

#endif
