#ifndef _A4_INPUT_STREAM_H_
#define _A4_INPUT_STREAM_H_

#include <string>
#include <vector>

#include <a4/types.h>

namespace google { 
namespace protobuf {
    class FileDescriptor;
}
}

namespace a4 {
namespace io {

    class InputStreamImpl;
    class StreamFooter;
    class A4Message;

    /// A4 Input Stream - reads protobuf messages from file

    /// A stream has "content message" (aka events) and metadata.
    /// Get the next non-metadata message by calling next(),
    /// after that you can get the current_metadata().
    /// Alternatively, call next(true)
    class InputStream
    {
        public:
            InputStream(std::string);
            InputStream(UNIQUE<InputStreamImpl>);
            virtual ~InputStream();

            /// Returns the next regular message in the stream.
            shared<A4Message> next();

            /// Explicitely end processing on this stream. Sets end() to true
            void close();

            /// Returns the next regular or metadata message.
            /// This is needed to process all metadata in streams where 
            /// metadata messages immediately follow each other.
            /// ATTENTION: Metadata messages are reordered so that they 
            /// always precede the data they refer to.
            shared<A4Message> next_with_metadata();

            /// \internal Return the next bare message (includes stream messages)
            shared<A4Message> next_bare_message();

            /// Return the currently applicable metadata message.
            shared<const A4Message> current_metadata();

            /// Skip the current block until the next metadata
            bool skip_to_next_metadata();

            /// True if new metadata has appeared since the last call to this function.
            bool new_metadata();

            /// True if the stream has not ended or encountered an error.
            bool good();

            /// True if the stream has encountered an error.
            bool error();

            /// True if the stream has finished without error.
            bool end();
            
            /// Number of bytes this InputStream has read
            size_t ByteCount();

            /// Human-readable string representation
            std::string str();
            
            /// Return a vector containing a vector of metadata per x.
            const std::vector<std::vector<shared<a4::io::A4Message>>>& all_metadata();
            
            const std::vector<StreamFooter>& footers();
            
            std::vector<const google::protobuf::FileDescriptor*> get_filedescriptors();
            
        private:
            bool _new_metadata;
            UNIQUE<InputStreamImpl> _impl;
    };

}
}

#endif
