#include <a4/string.h>

#if defined(__has_include)
    #if __has_include(<cxxabi.h>)
    #define A4_HAVE_DEMANGLING
    #include <cxxabi.h>
    #endif // __clang__
#else
    #if defined(__GLIBCXX__) || defined(__GLIBCPP__)
    #define A4_HAVE_DEMANGLING
    #include <cxxabi.h>
    #endif // __GNUC__
#endif

std::ostream& _string_internal::operator<<(std::ostream &sstr, const std::type_info& ti) {
    int status = 1;
    #ifdef A4_HAVE_DEMANGLING
    char* real_name = abi::__cxa_demangle(ti.name(), 0, 0, &status);
    #else
    char* real_name = NULL;
    status = 1;
    #endif
    
    if (status == 0) {
        sstr << real_name;
        free(real_name);
    } else
        sstr << ti.name();
    
    return sstr;
}
