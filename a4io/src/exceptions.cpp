#include <unistd.h>

#include <a4/exceptions.h>

#include <execinfo.h>
#include <signal.h>

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

#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

typedef boost::unique_lock<boost::recursive_mutex> Lock;
static boost::recursive_mutex segfault_mutex;
static boost::recursive_mutex exception_mutex;

namespace a4{

    BackTraceException::BackTraceException() {
        // Lock this backtrace generation
        Lock lock(exception_mutex);

        //std::cerr << "A4: An error occurred - trying to attach gdb to the current process --" << std::endl;
        // Try using GDB first...
        bool gdb_worked = true;
        FILE *fpipe;
        std::stringstream cmd1, cmd2, res1, res2, res3;
        cmd1 << "gdb -n --batch --eval-command='set complaints 0' --eval-command='attach " << getpid() << "' --eval-command='thread apply all bt' 2>&1";
        cmd2 << "gdb -n --batch --eval-command='set complaints 0' --eval-command='attach " << getpid() << "' --eval-command='thread apply all bt full' 2>&1";
        char line[256];
        if ( !(fpipe = (FILE*)popen(cmd1.str().c_str(),"r")) ) gdb_worked = false;
        else while (fgets( line, sizeof line, fpipe)) res1 << line;
        if (pclose(fpipe) != 0) gdb_worked = false;
        if ( !(fpipe = (FILE*)popen(cmd2.str().c_str(),"r")) ) gdb_worked = false;
        else while (fgets( line, sizeof line, fpipe)) res2 << line;
        if (pclose(fpipe) != 0) gdb_worked = false;

        res1.seekg(0);
        while(!res1.eof()) {
            std::string ln;
            std::getline(res1, ln);
            if(ln.substr(0, 31) == "ptrace: Operation not permitted.") {
                gdb_worked = false;
                break;
            }
            if(ln.substr(0, 7) == "warning:") continue;
            res3 << "GDB: " << ln << std::endl;
        }

        _backtrace = res3.str();
        _full_backtrace = res2.str();
        if (gdb_worked) return;
        //std::cerr << "A4: GDB failed to attach to the current process. Trying to extract backtrace manually... " << std::endl;

        // If GDB did not work...
        void* stack_funcs[50];
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

                int status = 1;
                #ifdef A4_HAVE_DEMANGLING
                char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
                #else
                char* real_name = mangled_name;
                #endif

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
        _full_backtrace = sstr.str();
    };

    void Fatal::handle_exception(std::string reason) {
        std::stringstream sstr;
        sstr << "A4 FATAL ERROR: " << reason << std::endl;
        sstr << "Backtrace:\n--------------------------------" << std::endl;
        sstr << _backtrace;
        sstr << "--------------------------------" << std::endl;
        sstr << "A4 FATAL ERROR: " << reason << std::endl;
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

    bool Fatal::enable_throw_on_segfault() {
        struct sigaction act;
        act.sa_handler = &segfault_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_RESETHAND | SA_NODEFER;
        bool success = (sigaction(SIGSEGV, &act, NULL) == 0);
        sigaction(SIGALRM, &act, NULL);
        return success;
    };

    void Fatal::segfault_handler(int i) {
        if (i == SIGALRM) {
            std::cerr << "GDB backtrace timed out. Please run the debugger manually to obtain a backtrace." << std::endl;
            std::terminate();
        }
        std::cerr << "A4 segfault handler called..." << std::endl;
        {
            Lock lock(segfault_mutex);
            if (segfault_handled) {
                std::cerr << "double segfault or segfault in exception handler - sorry!" << std::endl;
                std::terminate();
            }
            segfault_handled = true;
            // Schedule a 5-second timeout
            alarm(5);
        }

        throw Fatal("Segmentation Fault!");
    }

    volatile bool Fatal::segfault_handled = false;
};

