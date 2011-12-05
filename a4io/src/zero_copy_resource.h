#ifndef _A4_ZERO_COPY_RESOURCE_H_
#define _A4_ZERO_COPY_RESOURCE_H_

#include <string>

#include <sys/mman.h>
#include <sys/stat.h>

#include <google/protobuf/io/zero_copy_stream.h>

#include <a4/types.h>


namespace google{ namespace protobuf{ namespace io{ class FileInputStream; class CopyingInputStreamAdaptor; };};};

namespace a4{ namespace io{

    class ZeroCopyStreamResource : public google::protobuf::io::ZeroCopyInputStream {
        public:
            virtual ~ZeroCopyStreamResource() {};

            /// True if arbitrary seeks are allowed
            virtual bool seekable() const { return false; };

            /// Seek to the given position. Existing Next() buffers are invalidated.
            virtual bool Seek(size_t position) { return false; };

            /// Seek to the given position from the end of file. Existing Next() buffers are invalidated.
            virtual bool SeekBack(size_t position) {
                throw a4::Fatal("BUG! Tried to seek backwards in a stream. This doesn't make sense.");
            };
            
            virtual size_t Tell() const { return 0; };

            /// Get a clone of this stream with the given offset
            virtual unique<ZeroCopyStreamResource> Clone(size_t offset) {
                return unique<ZeroCopyStreamResource>();
            };
    };

    struct OpenFile {
        OpenFile(const char * name, int oflag, bool mmap=true);
        ~OpenFile();

        int no;
        size_t size;
        void* mmap;
        bool error;
    };

    /// Class that deals with the specifics of having a physical file on disk
    class UnixFileMMap : public ZeroCopyStreamResource {
        public:

            UnixFileMMap(std::string name);
            virtual ~UnixFileMMap();

            virtual bool open();
            virtual bool close();

            bool Next(const void** data, int* size);
            void BackUp(int count);

            bool Skip(int count);

            bool seekable() const { return true; };

            bool Seek(size_t position);
            bool SeekBack(size_t position);
            size_t Tell() const;
            google::protobuf::int64 ByteCount() const { return Tell(); };

            virtual unique<ZeroCopyStreamResource> Clone(size_t offset);

        protected:
            std::string _name;
            shared<OpenFile> _file;
            size_t _size;
            size_t _position;
            void* _mmap;
            bool _error;
            bool _open;
    };

    /// Class that deals with the specifics of having a physical file on disk
    class UnixFilePartMMap : public UnixFileMMap {
        public:
            UnixFilePartMMap(std::string name);
            bool open();
            bool close();
            bool Next(const void** data, int* size);
            virtual unique<ZeroCopyStreamResource> Clone(size_t offset);

        private:
            void remap();
            size_t _mmap_offset;
            size_t _mmap_blocksize;
    };

    class UnixStream : public ZeroCopyStreamResource {
        public:
            UnixStream();
            UnixStream(int file_descriptor, int block_size = -1);
            bool Next(const void** data, int* size);
            void BackUp(int count);
            bool Skip(int count);
            google::protobuf::int64 ByteCount() const;
              
        protected:
            unique<google::protobuf::io::FileInputStream> _impl;

    };

    class UnixFile : public UnixStream {
        public:
            UnixFile(std::string filename, int block_size = -1);
        protected:
            unique<OpenFile> _file;
    };
    
    class RemoteCopyingFile;
    class RemoteFile : public ZeroCopyStreamResource {
        public:
            RemoteFile(std::string filename, int block_size = -1);
            virtual bool seekable() const { return true; };
            bool Next(const void** data, int* size);
            void BackUp(int count);
            bool Skip(int count);
            bool Seek(size_t position);
            bool SeekBack(size_t position);
            size_t Tell() const;
            google::protobuf::int64 ByteCount() const;
        protected:
            void new_adaptor(bool tell=false) const;
            mutable unique<google::protobuf::io::CopyingInputStreamAdaptor> _adaptor;
            unique<RemoteCopyingFile> _file;
            std::string _filename;
            int _block_size;
            mutable uint64_t _byte_count;
    };

    unique<ZeroCopyStreamResource> resource_from_url(std::string url);

};};

#endif
