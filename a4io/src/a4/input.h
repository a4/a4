#ifndef _A4_INPUT_H_
#define _A4_INPUT_H_

#include <set>
#include <map>
#include <deque>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

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
            A4Input& add_stream(shared<InputStream>); 
            /// Add a file to be processed, Returns this object again.
            A4Input& add_file(const std::string& filename);

            /// Get a stream resource for processing,
            /// returns NULL if none are left (threadsafe).
            /// To return the resource just let this shared pointer go 
            /// out of scope, the stream will be returned and processed more
            /// if it is needed.
            shared<InputStream> get_stream();
        private:
            static void report_finished(A4Input *, InputStream* _s);
            InputStream* pop_file();
            std::deque<std::string> _filenames;
            std::vector<shared<InputStream>> _streams;
            std::deque<InputStream*> _ready;
            std::set<InputStream*> _processing;
            std::set<InputStream*> _finished;
            std::set<InputStream*> _error;
            std::map<InputStream*,int> _resched_count;
            mutable boost::mutex _mutex;
    };
}; };

#endif
