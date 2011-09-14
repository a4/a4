#ifndef _A4_INPUT_H_
#define _A4_INPUT_H_

#include <set>
#include <map>
#include <mutex>

#include <a4/input_stream.h>


namespace a4{ namespace io{

    /// Collects several Input streams and provide them on request.
    //
    /// This class represents one or more input streams, and hands them out,
    /// making sure that each is only processed once per A4Input lifetime.
    /// (Similar to an iterator)
    /// This class is thread-safe - several threads may request streams and 
    /// use them at the same time.
    class A4Input {
        public:
            /// Create Input, with optional name to distinguish multiple inputs 
            A4Input(std::string name="A4Input");
            /// Add a stream to be processed, Returns this object again.
            A4Input & add_stream(shared<A4InputStream>); 
            /// Add a file to be processed, Returns this object again.
            A4Input & add_file(std::string & filename);

            /// Get a stream resource for processing,
            /// returns NULL if none are left (threadsafe).
            /// Optionally give your "name" (for debugging)
            /// To return the resource just let this shared pointer go 
            /// out of scope, the stream will be returned and processed more
            /// if it is needed.
            shared<A4InputStream> get_stream(std::string me="");
        private:
            static void report_finished(A4Input *, A4InputStream* _s);
            std::vector<shared<A4InputStream>> _streams;
            std::vector<A4InputStream*> _ready;
            std::set<A4InputStream*> _processing;
            std::set<A4InputStream*> _finished;
            std::map<A4InputStream*, std::string> _names;

            mutable std::mutex _mutex;
    };
}; };

#endif
