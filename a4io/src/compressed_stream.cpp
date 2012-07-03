// Author: peter.waller@gmail.com (Peter Waller)
// Author: johannes@ebke.org (Johannes Ebke)

#include <a4/config.h>

#include <iostream>
#include <cmath>

#include <google/protobuf/stubs/common.h>

#include <google/protobuf/io/coded_stream.h>
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::CodedOutputStream;

#if HAVE_SNAPPY
#include <snappy.h>
#endif

#include "lz4.h"

#include "compressed_stream.h"

namespace a4 {
namespace io {

const size_t BLOCKSIZE = 64 * 1024;

GenericCompressionInputStream::GenericCompressionInputStream(
    ZeroCopyInputStream* sub_stream)
    : _sub_stream(NULL), _backed_up_bytes(0), _byte_count(0) {
    _raw_stream = sub_stream;
    _sub_stream = new CodedInputStream(_raw_stream);
    _sub_stream->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
}

GenericCompressionInputStream::~GenericCompressionInputStream() {
    delete _sub_stream;
}

bool GenericCompressionInputStream::Skip(int count) {
    assert(false);
};

void GenericCompressionInputStream::BackUp(int count) {
    _backed_up_bytes += count;
};

void GenericCompressionInputStream::reset_input_stream() {
    delete _sub_stream;
    _sub_stream = new CodedInputStream(_raw_stream);
    _sub_stream->SetTotalBytesLimit(pow(1024,3), pow(1024,3));
}

bool GenericCompressionInputStream::Next(const void** data, int* size) {
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
    shared<char> tempbuffer(new char[compressed_size], array_delete<char>()); 
    _sub_stream->ReadRaw(tempbuffer.get(), compressed_size);
    
    RawUncompress(tempbuffer.get(), compressed_size);
    
    reset_input_stream(); // TODO(ebke): probably call this every Limit/BLOCKSIZE
    
    (*size) = _output_buffer_size;
    (*data) = _output_buffer.get();
    return true;
}

// =========================================================================

GenericCompressionOutputStream::GenericCompressionOutputStream(
    ZeroCopyOutputStream* sub_stream)
    : _sub_stream(new CodedOutputStream(sub_stream)), _backed_up_bytes(0),
      _byte_count(0) {}

GenericCompressionOutputStream::~GenericCompressionOutputStream() {
    if (_input_buffer) {
        // The buffer is not empty, there is stuff yet to be written.
        // This is necessary because we can't call virtual functions from any
        // destructor. Often this results in a pure virtual function call.
        FATAL("Call GenericCompressionOutputStream::Flush() before destroying "
              "this object");
        // Flush();
    }
    delete _sub_stream;
}

void GenericCompressionOutputStream::BackUp(int count) {
    _backed_up_bytes += count;
}

bool GenericCompressionOutputStream::Flush()
{
    size_t size = BLOCKSIZE - _backed_up_bytes;
    if (!_input_buffer || size == 0) return true;
    
    size_t compressed_size = 0;
    shared<char> compressed_data(new char[MaxCompressedLength(size)], array_delete<char>());
    compressed_size = RawCompress(_input_buffer.get(), size, compressed_data.get());
    
    assert(compressed_size <= 2*BLOCKSIZE);
    
    uint32_t compressed_size_32 = static_cast<uint32_t>(compressed_size);
    _sub_stream->WriteVarint32(compressed_size_32);
    _sub_stream->WriteRaw(compressed_data.get(), compressed_size_32);
        
    _backed_up_bytes = 0;
    _input_buffer.reset();
    return true;
}

bool GenericCompressionOutputStream::Next(void** data, int* size) {
    if (_backed_up_bytes) {
        size_t skip = BLOCKSIZE - _backed_up_bytes;
        assert(skip >= 0);
        (*data) = _input_buffer.get() + skip;
        (*size) = _backed_up_bytes;
        _backed_up_bytes = 0;
        return true;
    }
    if(_input_buffer) Flush();
    _input_buffer.reset(new char[BLOCKSIZE], array_delete<char>());
    (*data) = _input_buffer.get();
    (*size) = BLOCKSIZE;
    return true;
}



#if HAVE_SNAPPY

/// Snappy implementation

void SnappyInputStream::RawUncompress(char* input_buffer, uint32_t compressed_size) {
    bool success = ::snappy::GetUncompressedLength(
        input_buffer, compressed_size, &_output_buffer_size);
    assert(success);
    
    _output_buffer.reset(new char[_output_buffer_size], array_delete<char>());
    success = ::snappy::RawUncompress(input_buffer, compressed_size,
        _output_buffer.get());
    assert(success);    
}


uint32_t SnappyOutputStream::MaxCompressedLength(size_t input_size)
{
    return ::snappy::MaxCompressedLength(input_size);
}

uint32_t SnappyOutputStream::RawCompress(char* input_buffer, size_t input_size,
    char* output_buffer)
{
    size_t compressed_size = 0;
    ::snappy::RawCompress(input_buffer, input_size, output_buffer,
        &compressed_size);
    return compressed_size;
}

#endif  // HAVE_SNAPPY

/// LZ4 implementation

void LZ4InputStream::RawUncompress(char* input_buffer, uint32_t compressed_size) {
    _output_buffer.reset(new char[BLOCKSIZE], array_delete<char>());
    _output_buffer_size = LZ4_uncompress_unknownOutputSize(input_buffer,
        _output_buffer.get(), compressed_size, BLOCKSIZE);
}

uint32_t LZ4OutputStream::MaxCompressedLength(size_t input_size)
{
    return LZ4_compressBound(input_size);
}

uint32_t LZ4OutputStream::RawCompress(char* input_buffer, size_t input_size,
    char* output_buffer)
{
    assert(input_size);
    return ::LZ4_compress(input_buffer, output_buffer, input_size);
}

}  // namespace io
}  // namespace a4

