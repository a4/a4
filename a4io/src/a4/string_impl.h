#include <a4/string.h>
#include <typeinfo>
#include <cxxabi.h>

namespace _string_internal {
    static std::ostream& operator<<(std::ostream &sstr, const std::type_info& ti) {
        int status;
        char * real_name = abi::__cxa_demangle(ti.name(), 0, 0, &status);
        if (real_name) sstr << real_name; else sstr << ti.name();
        free(real_name);
        return sstr;
    };

    static inline std::string str_printf(const char * s) {
        return std::string (s);
    };

    template<typename T, typename... Args>
    std::string str_printf(const char * s, const T& value, const Args&... args) {
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
    };

    static inline std::string str_cat() { return std::string(""); };

    template<typename T, typename... Args>
    std::string str_cat(const T& s, const Args&... args) {
        std::stringstream ss;
        ss << s << str_cat(args...);
        return ss.str();
    };

};

template<typename... Args>
static inline std::string str_printf(const char * s, const Args&... args) {
    return _string_internal::str_printf(s, args...);
}

template<typename... Args>
static inline std::string str_cat(const Args&... args) {
    return _string_internal::str_cat(args...);
};

