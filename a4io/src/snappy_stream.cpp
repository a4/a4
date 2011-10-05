// Author: peter.waller@gmail.com (Peter Waller)
// Author: johannes@ebke.org (Johannes Ebke)

#include "config.h"

#if HAVE_SNAPPY

#include <google/protobuf/stubs/common.h>

#include <google/protobuf/io/coded_stream.h>
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::CodedOutputStream;

#include <snappy.h>

#include "snappy_stream.h"

namespace a4 {
namespace io {

const size_t BLOCKSIZE = 64 * 1024;

SnappyInputStream::SnappyInputStream(ZeroCopyInputStream* sub_stream)
    : _sub_stream(new CodedInputStream(sub_stream)), _byte_count(0) {

}
SnappyInputStream::~SnappyInputStream() {
}

bool SnappyInputStream::Skip(int count) {
    assert(false);
};

void SnappyInputStream::BackUp(int count) {
    _backed_up_bytes += count;
};


bool SnappyInputStream::Next(const void** data, int* size) {
    if (_backed_up_bytes) {
       size_t skip = _output_buffer_size - _backed_up_bytes;
       assert(skip >= 0);
       (*data) = _output_buffer.get() + skip;
       (*size) = _backed_up_bytes;
       _backed_up_bytes = 0;
       return true;
    }
    
    uint32_t compressed_size = 0;
    assert(_sub_stream->ReadVarint32(&compressed_size));
    assert(compressed_size < BLOCKSIZE*10);
    shared<char> tempbuffer(new char[compressed_size]); 
    _sub_stream->ReadRaw(tempbuffer.get(), compressed_size);
    
    bool success = snappy::GetUncompressedLength(
        tempbuffer.get(), compressed_size, &_output_buffer_size);
    assert(success);
    
    success = snappy::RawUncompress(tempbuffer.get(), compressed_size, _output_buffer.get());
    assert(success);
    _output_buffer.reset(new char[_output_buffer_size]);
    
    (*size) = _output_buffer_size;
    (*data) = _output_buffer.get();
    return true;
}

// =========================================================================

SnappyOutputStream::SnappyOutputStream(ZeroCopyOutputStream* sub_stream) 
    : _sub_stream(new CodedOutputStream(sub_stream)), _byte_count(0) {
}

void SnappyOutputStream::BackUp(int count) {
    _backed_up_bytes += count;
}

void SnappyOutputStream::Flush()
{
    size_t size = BLOCKSIZE - _backed_up_bytes;
    
    size_t compressed_size = 0;
    shared<char> compressed_data(new char[snappy::MaxCompressedLength(size)]);
    snappy::RawCompress(_input_buffer.get(), size, compressed_data.get(), &compressed_size);
    
    assert(compressed_size <= 2*BLOCKSIZE);
    
    uint32_t compressed_size_32 = static_cast<uint32_t>(compressed_size);
    _sub_stream->WriteVarint32(compressed_size_32);
    _sub_stream->WriteRaw(compressed_data.get(), compressed_size_32);
        
    _input_buffer.reset();
}

bool SnappyOutputStream::Next(void** data, int* size) {
    if (_backed_up_bytes) {
       size_t skip = BLOCKSIZE - _backed_up_bytes;
       assert(skip >= 0);
       (*data) = _input_buffer.get() + skip;
       (*size) = _backed_up_bytes;
       _backed_up_bytes = 0;
       return true;
    }
    if(_input_buffer) Flush();
    _input_buffer.reset(new char[BLOCKSIZE]);
    (*data) = _input_buffer.get();
    (*size) = BLOCKSIZE;
    return true;
}

}  // namespace io
}  // namespace a4

#endif  // HAVE_SNAPPY
