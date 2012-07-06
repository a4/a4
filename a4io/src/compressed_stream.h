// Author: peter.waller@gmail.com (Peter Waller)
// Author: johannes@ebke.org (Johannes Ebke)

#ifndef A4_IO_COMPRESSED_STREAM_H__
#define A4_IO_COMPRESSED_STREAM_H__

#include <google/protobuf/io/zero_copy_stream.h>
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::ZeroCopyOutputStream;

#include <google/protobuf/io/coded_stream.h>
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::CodedOutputStream;

#include <a4/types.h>
#include <base_compressed_streams.h>

namespace a4 {
namespace io {

// A ZeroCopyInputStream that reads compressed data through GenericCompression
class GenericCompressionInputStream : public BaseCompressedInputStream {
 public:

  explicit GenericCompressionInputStream(ZeroCopyInputStream* sub_stream);
  virtual ~GenericCompressionInputStream();
  
  void reset_input_stream();
  
  // implements ZeroCopyInputStream ----------------------------------
  bool Next(const void** data, int* size);
  void BackUp(int count);
  bool Skip(int count);
  int64_t ByteCount() const { return _byte_count; }

  virtual void RawUncompress(char* input_buffer, uint32_t compressed_size) = 0;
  
 protected:
  shared<char> _output_buffer;
  size_t _output_buffer_size;
  
 private:

  CodedInputStream* _sub_stream;
  ZeroCopyInputStream* _raw_stream;

  int _backed_up_bytes;

  size_t _byte_count;
  
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(GenericCompressionInputStream);
};

class GenericCompressionOutputStream : public BaseCompressedOutputStream {
 public:
  // Create a GenericCompressionOutputStream with default options.
  explicit GenericCompressionOutputStream(ZeroCopyOutputStream* sub_stream);

  virtual ~GenericCompressionOutputStream();
  
  // implements ZeroCopyOutputStream ---------------------------------
  bool Next(void** data, int* size);
  void BackUp(int count);
  int64_t ByteCount() const { return _byte_count; };

  bool Flush();
  bool Close() {return true; }
  
  virtual uint32_t MaxCompressedLength(size_t input_size) = 0;
  virtual uint32_t RawCompress(char* input_buffer, size_t input_size, char* output_buffer) = 0;

 protected:
  shared<char> _input_buffer;
  size_t _input_buffer_size;
   
 private:
  
  CodedOutputStream* _sub_stream;
  
  int _backed_up_bytes;
  
  size_t _byte_count;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(GenericCompressionOutputStream);
};

#ifdef HAVE_SNAPPY

class SnappyInputStream : public GenericCompressionInputStream {
 public:

  explicit SnappyInputStream(ZeroCopyInputStream* sub_stream) : GenericCompressionInputStream(sub_stream) {};
  
  virtual void RawUncompress(char* input_buffer, uint32_t compressed_size);
};

class SnappyOutputStream : public GenericCompressionOutputStream {
 public:
  explicit SnappyOutputStream(ZeroCopyOutputStream* sub_stream) : GenericCompressionOutputStream(sub_stream) {};
  
  virtual uint32_t MaxCompressedLength(size_t input_size);
  virtual uint32_t RawCompress(char* input_buffer, size_t input_size, char* output_buffer);
  
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(SnappyOutputStream);
};

#endif

class LZ4InputStream : public GenericCompressionInputStream {
 public:

  explicit LZ4InputStream(ZeroCopyInputStream* sub_stream) : GenericCompressionInputStream(sub_stream) {};
  
  virtual void RawUncompress(char* input_buffer, uint32_t compressed_size);
  
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(LZ4InputStream);
};

class LZ4OutputStream : public GenericCompressionOutputStream {
 public:
  explicit LZ4OutputStream(ZeroCopyOutputStream* sub_stream) : GenericCompressionOutputStream(sub_stream) {};
  
  virtual uint32_t MaxCompressedLength(size_t input_size);
  virtual uint32_t RawCompress(char* input_buffer, size_t input_size, char* output_buffer);
  
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(LZ4OutputStream);
};

}  // namespace io
}  // namespace protobuf

#endif  // A4_IO_SNAPPY_STREAM_H__
