#ifndef _A4_BASE_COMPRESSED_STREAMS_H_
#define _A4_BASE_COMPRESSED_STREAMS_H_

#include <google/protobuf/io/zero_copy_stream.h>

namespace a4 {
namespace io {

class BaseCompressedOutputStream : public google::protobuf::io::ZeroCopyOutputStream {
public:    
    virtual bool Flush() = 0;
    virtual bool Close() {return true;};
};


class BaseCompressedInputStream : public google::protobuf::io::ZeroCopyInputStream {
public:
    bool ExpectAtEnd() { return true; }
};

}
}

#endif
