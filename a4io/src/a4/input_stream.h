#ifndef _A4_INPUT_STREAM_H_
#define _A4_INPUT_STREAM_H_

#include <string>

#include <a4/types.h>
#include <a4/message.h>

namespace a4{ namespace io{

    class InputStreamImpl;
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
            InputStream(unique<InputStreamImpl>);
            virtual ~InputStream();

            /// Returns the next regular message in the stream.
            A4Message next();

            /// Returns the next regular or metadata message.
            /// This is needed to process all metadata in streams where 
            /// metadata messages immediately follow each other.
            /// ATTENTION: Metadata messages are reordered so that they 
            /// always precede the data they refer to.
            A4Message next_with_metadata();

            /// \internal Return the next bare message (includes stream messages)
            A4Message next_bare_message();

            /// Return the currently applicable metadata message.
            const A4Message current_metadata();

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
            const std::vector<std::vector<a4::io::A4Message>>& all_metadata();
            
        private:
            bool _new_metadata;
            unique<InputStreamImpl> _impl;
    };

};};

#endif
