#ifndef _A4_EXCEPTIONS_
#define _A4_EXCEPTIONS_

#include <exception>

#include <a4/string.h>

namespace a4 {

    /// Exception base class that collects a backtrace
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
    
    /// Exception for Fatal, unexpected errors
    class Fatal : public BackTraceException {
        public:
            template<typename ...Args>
            Fatal(const Args&... args) {
                handle_exception(str_cat(args...));
            };
            virtual ~Fatal() throw() {};

            void handle_exception(std::string);

            virtual const char* what() const throw() {
                return _what.c_str();
            }
            static bool enable_throw_on_segfault();
            static void segfault_handler(int i);

        protected:
            static volatile bool segfault_handled;
            std::string _what;
    };
    
    /// Exception for "expected" errors that should terminate
    /// the program ("file not found" or similar)
    class Terminate : public std::exception {
        public:
            template<typename ...Args>
            Terminate(const Args&... args) {
                handle_exception(str_cat(args...));
            };
            virtual ~Terminate() throw() {};

            void handle_exception(std::string);

            virtual const char* what() const throw() {
                return _what.c_str();
            }
        protected:
            std::string _what;
    };

    
}

#endif
