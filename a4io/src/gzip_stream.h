// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: brianolson@google.com (Brian Olson)
//
// This file contains the definition for classes GzipInputStream and
// GzipOutputStream.
//
// GzipInputStream decompresses data from an underlying
// ZeroCopyInputStream and provides the decompressed data as a
// ZeroCopyInputStream.
//
// GzipOutputStream is an ZeroCopyOutputStream that compresses data to
// an underlying ZeroCopyOutputStream.

#include <zlib.h>

#include <google/protobuf/io/zero_copy_stream.h>

namespace a4 {
namespace io {

using google::protobuf::int64;

// A ZeroCopyInputStream that reads compressed data through zlib
class GzipInputStream : public google::protobuf::io::ZeroCopyInputStream {

 public:
  // Format key for constructor
  enum Format {
    // zlib will autodetect gzip header or deflate stream
    AUTO = 0,

    // GZIP streams have some extra header data for file attributes.
    GZIP = 1,

    // Simpler zlib stream format.
    ZLIB = 2,
  };

  // buffer_size and format may be -1 for default of 64kB and GZIP format
  explicit GzipInputStream(
      ZeroCopyInputStream* sub_stream,
      Format format = AUTO,
      int buffer_size = -1);
  virtual ~GzipInputStream();

  // Return last error message or NULL if no error.
  inline const char* ZlibErrorMessage() const {
    return zcontext_.msg;
  }
  inline int ZlibErrorCode() const {
    return zerror_;
  }

  // Returns true if the compressed stream is at an end
  // Reads zlib footer, so one can continue reading the
  // underlying stream after this stream is deleted.
  bool ExpectAtEnd();

  // implements ZeroCopyInputStream ----------------------------------
  bool Next(const void** data, int* size);
  void BackUp(int count);
  bool Skip(int count);
  int64 ByteCount() const;

 private:
  Format format_;

  ZeroCopyInputStream* sub_stream_;

  z_stream zcontext_;
  int zerror_;

  void* output_buffer_;
  void* output_position_;
  size_t output_buffer_length_;

  int Inflate(int flush);
  void DoNextOutput(const void** data, int* size);

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(GzipInputStream);
};

}  // namespace io
}  // namespace a4

