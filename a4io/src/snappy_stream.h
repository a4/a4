// Author: peter.waller@gmail.com (Peter Waller)
// Author: johannes@ebke.org (Johannes Ebke)

#ifndef A4_IO_SNAPPY_STREAM_H__
#define A4_IO_SNAPPY_STREAM_H__

#include <snappy.h>

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

// A ZeroCopyInputStream that reads compressed data through Snappy
class SnappyInputStream : public BaseCompressedInputStream {
 public:

  explicit SnappyInputStream(ZeroCopyInputStream* sub_stream);
  virtual ~SnappyInputStream();
  
  void reset_input_stream();
  
  // implements ZeroCopyInputStream ----------------------------------
  bool Next(const void** data, int* size);
  void BackUp(int count);
  bool Skip(int count);
  int64_t ByteCount() const { return _byte_count; };

  virtual void RawUncompress(char* input_buffer, size_t compressed_size);

 private:

  CodedInputStream* _sub_stream;
  ZeroCopyInputStream* _raw_stream;

  int _backed_up_bytes;
  shared<char> _output_buffer;
  size_t _output_buffer_size;

  size_t _byte_count;
  
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(SnappyInputStream);
};

class SnappyOutputStream : public BaseCompressedOutputStream {
 public:
  // Create a SnappyOutputStream with default options.
  explicit SnappyOutputStream(ZeroCopyOutputStream* sub_stream);

  virtual ~SnappyOutputStream();
  
  // implements ZeroCopyOutputStream ---------------------------------
  bool Next(void** data, int* size);
  void BackUp(int count);
  int64_t ByteCount() const { return _byte_count; };

  bool Flush();
  bool Close() {return true; }
  
  virtual size_t MaxCompressedLength(size_t input_size);
  virtual size_t RawCompress(char* input_buffer, size_t input_size, char* output_buffer);
  
 private:
  
  CodedOutputStream* _sub_stream;
  
  int _backed_up_bytes;
  shared<char> _input_buffer;
  size_t _input_buffer_size;
  
  size_t _byte_count;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(SnappyOutputStream);
};

}  // namespace io
}  // namespace protobuf

#endif  // A4_IO_SNAPPY_STREAM_H__
