#ifndef _A4_EXCEPTIONS_
#define _A4_EXCEPTIONS_

#include <execinfo.h>
#include <signal.h>

#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cxxabi.h>
#include <cstdlib>

#include <boost/filesystem.hpp>

#include <a4/string.h>

namespace a4{

    class BackTraceException : public std::exception {
        public:
            BackTraceException() {
                void * stack_funcs[50];
                int stack_size = backtrace(stack_funcs, 50);
                char ** symbols = backtrace_symbols(stack_funcs, stack_size);
                std::stringstream sstr;

                // skip first stack frame (points here)
                for (int i = 1; i < stack_size && symbols != NULL; i++) {

                    // find parantheses and +address offset surrounding mangled name
                    char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;
                    for (char *p = symbols[i]; *p; p++) {
                        if (*p == '(') {
                            mangled_name = p; 
                        } else if (*p == '+') {
                            offset_begin = p;
                        } else if (*p == ')') {
                            offset_end = p;
                            break;
                        }
                    }

                    // if the line could be processed, attempt to demangle the symbol
                    if (mangled_name && offset_begin && offset_end && 
                        mangled_name < offset_begin) {
                        *mangled_name++ = '\0';
                        *offset_begin++ = '\0';
                        *offset_end++ = '\0';

                        int status;
                        char * real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

                        // if demangling is successful, output the demangled function name
                        if (status == 0) {    
                            sstr << "[" << i << "] " << symbols[i] << " : " 
                                      << real_name << "+" << offset_begin << offset_end 
                                      << std::endl;
                        } else {
                        // otherwise, output the mangled function name
                            sstr << "[" << i << "] " << symbols[i] << " : " 
                                      << mangled_name << "+" << offset_begin << offset_end 
                                      << std::endl;
                        }
                        free(real_name);
                    } else {
                    // otherwise, print the whole line
                        sstr << "[" << i << "] " << symbols[i] << std::endl;
                    }
                }
                free(symbols);
                _backtrace = sstr.str();
            };
            virtual ~BackTraceException() throw() {};
            virtual const char* what() const throw() {
                return _backtrace.c_str();
            }
        protected:
            std::string _backtrace;
    };
    class Fatal : public BackTraceException {
        public:
            template<typename ...Args>
            Fatal(const Args&... args) {
                std::stringstream sstr;
                sstr << "A4 FATAL ERROR: " << str_cat(args...) << std::endl;
                sstr << "Backtrace:\n--------------------------------" << std::endl;
                sstr << _backtrace;
                sstr << "--------------------------------" << std::endl;
                sstr << "A4 FATAL ERROR: " << str_cat(args...) << std::endl;
                // Write to $XDG_CACHE_HOME/a4/last-error or $(HOME)/.cache/a4/last-error
                try {
                    using boost::filesystem::path; using boost::filesystem::create_directories;
                    path dirname;
                    // http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
                    if (getenv("XDG_CACHE_HOME")) {
                        dirname = getenv("XDG_CACHE_HOME");
                    } else {
                        dirname = path(getenv("HOME")) / ".cache" / "a4";
                    }
                    create_directories(dirname);
                    dirname /= "last-fatal-error";
                    std::ofstream out(dirname.string());
                    out << sstr.str() << std::endl;
                    sstr << "\nNotice: Error has been written to " << dirname.string() << std::endl;
                } catch (...) {};
                sstr << "If this is a bug in A4, please submit an issue at https://github.com/JohannesEbke/a4/issues" << std::endl;
                _what = sstr.str();
            }
            virtual ~Fatal() throw() {};
            virtual const char* what() const throw() {
                return _what.c_str();
            }

            static bool enable_throw_on_segfault() {
                const struct sigaction act = {&segfault_handler, 0, 0, 0};
                sigaction(SIGSEGV, &act, NULL);
            };

            static void segfault_handler(int i) {
                throw Fatal("Segmentation Fault!");
            };

        protected:
            std::string _what;
    };

    
};

#endif
