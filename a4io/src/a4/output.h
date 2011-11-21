#ifndef _A4_OUTPUT_H_
#define _A4_OUTPUT_H_

#include <vector>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include <a4/output_stream.h>


namespace a4{ namespace io{

    /// Provides output streams to threads, and makes sure they are merged eventually.
    //
    /// This class represents one or more input streams, and hands them out,
    /// making sure that each is only processed once per A4Input lifetime.
    /// (Similar to an iterator)
    /// This class is thread-safe - several threads may request streams and 
    /// use them at the same time.
    class A4Output {
        public:
            /// Eventually write to output file
            A4Output(std::string output_file, std::string description);
            ~A4Output();
            /// Make sure all output is written and merge files
            bool close();
            /// Get a stream to write to (threadsafe).
            shared<OutputStream> get_stream(std::string postfix="");
        private:
            static void report_finished(A4Output *, OutputStream* _s);
            bool concatenate(const std::vector<std::string> & filenames, const std::string target);
            bool _closed;
            bool _regular_file;
            std::string _output_file;
            std::string _description;
            std::map<std::string, std::vector<shared<OutputStream>>> _out_streams;
            std::map<std::string, std::vector<std::string>> _filenames;
            mutable boost::mutex _mutex;
    };
};};

#endif
