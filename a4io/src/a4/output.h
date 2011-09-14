#ifndef _A4_OUTPUT_H_
#define _A4_OUTPUT_H_

#include <a4/output_stream.h>
#include <vector>

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
            /// Make sure all output is written and merge files
            bool close();
            /// Get a stream to write to (threadsafe).
            /// Accepts most of the same parameters as A4OutputStream
            shared<A4OutputStream> get_stream(
                uint32_t content_class_id,
                uint32_t metadata_class_id,
                bool metadata_refers_forward,
                bool compression);
        private:
            std::string output_file;
            std::string description;
            std::vector<shared<A4OutputStream>> _out_streams;
            std::vector<std::string> _filenames;
    };
};};

#endif
