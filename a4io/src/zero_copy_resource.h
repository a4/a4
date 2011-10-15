#ifndef _A4_ZERO_COPY_RESOURCE_H_
#define _A4_ZERO_COPY_RESOURCE_H_

#include <string>

#include <sys/mman.h>
#include <sys/stat.h>

#include <google/protobuf/io/zero_copy_stream.h>

#include <a4/types.h>

namespace a4{ namespace io{

    class ZeroCopyStreamResource : public google::protobuf::io::ZeroCopyInputStream {
        public:
            /// True if arbitrary seeks are allowed
            virtual bool seekable() const { return false; };

            /// Seek to the given position. Existing Next() buffers are invalidated.
            virtual bool Seek(size_t position) { return false; };

            /// Seek to the given position from the end of file. Existing Next() buffers are invalidated.
            virtual bool SeekBack(size_t position) { return false; };
            
            virtual size_t Tell() const { return 0; };

            /// Get a clone of this stream with the given offset
            virtual unique<ZeroCopyStreamResource> Clone(size_t offset) {
                return unique<ZeroCopyStreamResource>();
            };
    };

    struct OpenFile {
        OpenFile(const char * name, int oflag);
        ~OpenFile();

        int no;
        size_t size;
        void* mmap;
        bool error;
    };

    /// Class that deals with the specifics of having a physical file on disk
    class UnixFile : public ZeroCopyStreamResource {
        public:

            UnixFile(std::string name);
            virtual ~UnixFile();

            bool open();
            bool close();

            bool Next(const void** data, int* size);
            void BackUp(int count);

            bool Skip(int count);

            bool seekable() const { return true; };

            bool Seek(size_t position);
            bool SeekBack(size_t position);
            size_t Tell() const;
            google::protobuf::int64 ByteCount() const { return Tell(); };

            virtual unique<ZeroCopyStreamResource> Clone(size_t offset);

        private:
            std::string _name;
            shared<OpenFile> _file;
            size_t _size;
            size_t _position;
            void* _mmap;
            bool _error;
            bool _open;
    };

    unique<ZeroCopyStreamResource> resource_from_url(std::string url);

};};

#endif
