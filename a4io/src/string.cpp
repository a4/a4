#include <a4/string.h>

std::ostream& _string_internal::operator<<(std::ostream &sstr, const std::type_info& ti) {
    int status;
    char * real_name = abi::__cxa_demangle(ti.name(), 0, 0, &status);
    if (real_name) sstr << real_name; else sstr << ti.name();
    free(real_name);
    return sstr;
};
