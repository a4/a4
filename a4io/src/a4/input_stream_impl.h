#ifndef _A4_INPUT_STREAM_IMPL_
#define _A4_INPUT_STREAM_IMPL_

#include <a4/types.h>
#include <a4/io/A4Stream.pb.h>

#include <tuple>
#include <unordered_set>
#include <vector>

namespace google {
namespace protobuf {

class Descriptor;
class FileDescriptor;
class Message;

}
}

namespace a4 {
namespace io {

class A4Message;

class InputStreamImpl
{
    public:
        virtual ~InputStreamImpl() {};

        /// Returns the next regular message in the stream.
        virtual shared<A4Message> next(bool skip_metadata=true) = 0;
        /// Returns the next bare message in the stream.
        virtual shared<A4Message> next_bare_message() = 0;
        /// Returns the next regular or metadata message in the stream.
        virtual shared<A4Message> next_with_metadata() = 0;
        /// Return the current metadata message.
        virtual shared<const A4Message> current_metadata() = 0;
        /// Seek to the given header/metadata combination.
        /// If carry==false, specifying a metadata index not in that header section
        /// causes an exception, otherwise the next header is used, or false is returned on EOF.
        virtual bool seek_to(uint32_t header, int32_t metadata, bool carry=false) = 0;
        /// Skip to the start of the next metadata block. Return false if EOF, true if not.
        virtual bool skip_to_next_metadata() = 0;
        /// True if new metadata has appeared since the last call to this function.
        virtual bool new_metadata() = 0;
        /// True if the stream has not ended or encountered an error.
        virtual bool good() = 0;
        /// True if the stream has encountered an error.
        virtual bool error() = 0;
        /// True if the stream has finished without error.
        virtual bool end() = 0;
        /// explicitly close the stream
        virtual void close() = 0;
        
        virtual size_t ByteCount() = 0;

        virtual std::string str() = 0;

        virtual const std::vector<std::vector<shared<a4::io::A4Message>>>& all_metadata() = 0;
        virtual const std::vector<StreamFooter>& footers() = 0;
        virtual std::vector<const google::protobuf::FileDescriptor*> get_filedescriptors() = 0;
        virtual void set_hint_copy(bool hint_copy) = 0;
        virtual bool try_read(google::protobuf::Message & msg, const google::protobuf::Descriptor* d) = 0;
};

}
}

#endif
