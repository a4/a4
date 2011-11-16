#ifndef _A4_EXCEPTIONS_
#define _A4_EXCEPTIONS_

#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <boost/filesystem.hpp>

#include <a4/string.h>

namespace a4{

    class BackTraceException : public std::exception {
        public:
            BackTraceException();
            virtual ~BackTraceException() throw() {};
            virtual const char* what() const throw() {
                return _backtrace.c_str();
            }
        protected:
            static volatile bool called;
            std::string _backtrace;
            std::string _full_backtrace;
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
                    std::ofstream out(dirname.string().c_str());
                    out << sstr.str() << std::endl << "FULL BACKTRACE:" << std::endl << _full_backtrace << std::endl;
                    sstr << "\nNotice: Error has been written to " << dirname.string() << std::endl;
                } catch (...) {};
                sstr << "If this is a bug in A4, please submit an issue at https://github.com/JohannesEbke/a4/issues" << std::endl;
                _what = sstr.str();
            }

            virtual ~Fatal() throw() {};

            virtual const char* what() const throw() {
                return _what.c_str();
            }

            static bool enable_throw_on_segfault();

            static void segfault_handler(int i) {
                throw Fatal("Segmentation Fault!");
            };

        protected:
            std::string _what;
    };

    
};

#endif
