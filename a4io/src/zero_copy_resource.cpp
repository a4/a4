#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>

#include <boost/algorithm/string.hpp>

#include "zero_copy_resource.h"
#include "remote_io.h" 
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;

namespace a4{ namespace io{

    const uint32_t default_network_block_size = 32*1024;

    OpenFile::OpenFile(const char * name, int oflag, bool do_mmap) {
        error = true;
        no = size = 0;
        mmap = NULL;
        no = open(name, oflag);
        if (no < 0) {
            std::cerr << "ERROR - Could not open '" << name \
                      << "' - error: " << strerror(errno) << std::endl;
            return;
        }
        if (!do_mmap) {
            error = false;
            return;
        }
        struct stat buffer;
        static_assert(sizeof(buffer.st_size) >= sizeof(size_t), "OS Size information is 32 bit!");
// See https://github.com/JohannesEbke/a4/issues/34
//        static_assert(sizeof(size_t) >= sizeof(uint64_t), "size_t is 32 bit!");
        if (fstat(no, &buffer) == -1) {
            close(no);
            return;
        }
        size = buffer.st_size;
        mmap = ::mmap(NULL, size, PROT_READ, MAP_PRIVATE, no, 0);
        if (mmap == MAP_FAILED) {
            std::cerr << "ERROR - Could not mmap '" << name << "': " << strerror(errno) << std::endl;
            close(no);
            return;
        }
        error = false;
    }

    OpenFile::~OpenFile() { 
        if (error) return;
        error = true;
        if (mmap) munmap(mmap, size);
        close(no);
    }

    UnixFileMMap::UnixFileMMap(std::string name) : _name(name), _size(0), _position(0), _mmap(0), _error(false), _open(false) {};

    UnixFileMMap::~UnixFileMMap() { if (!_error) close(); };

    bool UnixFileMMap::open() {
        if (_error) return false;
        if (_open) { _error = true; return false; }

        _file.reset(new OpenFile(_name.c_str(), O_RDONLY, true));
        if (_file->error) {
            std::cerr << "ERROR - Could not open '" << _name \
                      << "' - error: " << strerror(errno) << std::endl;
            _error = true;
            return false;
        }
        _size = _file->size;
        _mmap = _file->mmap;
        _open = true;
        return true;
    }

    bool UnixFileMMap::close() {
        _file.reset();
        _error = true;
        return true;
    }

    bool UnixFileMMap::Next(const void** data, int* size) {
        if (_error) return false;
        if (!_open && !open()) return false;
        if (_size == _position) return false;
        *data = (uint8_t*)_mmap + _position;
        if ((_size - _position) > INT_MAX) {
            *size = INT_MAX;
        } else {
            *size = _size - _position;
        }
        _position += *size;
        return true;
    }

    void UnixFileMMap::BackUp(int count) {
        assert(count >= 0 && size_t(count) <= _position);
        _position -= size_t(count);
    }

    bool UnixFileMMap::Skip(int count) {
        if (count < 0) return false;
        _position += count;
        if (_position > _size) {
            _position = _size;
            return false;
        }
        return true;            
    }

    bool UnixFileMMap::Seek(size_t position) {
        if (_error) return false;
        if (!_open && !open()) return false;
        if (position > _size) return false;
        _position = position;
        return true;
    }

    bool UnixFileMMap::SeekBack(size_t position) {
        if (position > _size) return false;
        return Seek(_size - position);
    };

    size_t UnixFileMMap::Tell() const {
        return _position;
    }

    // Get a clone of this stream with the given offset
    unique<ZeroCopyStreamResource> UnixFileMMap::Clone(size_t offset) {
        assert(_open);
        unique<UnixFileMMap> f(new UnixFileMMap(_name));
        f->_file = _file;
        f->_position = offset;
        f->_mmap = (uint8_t*)_mmap + offset;
        f->_error = _error;
        f->_open = _open;
        return std::move(f);
    };

    UnixFilePartMMap::UnixFilePartMMap(std::string name) : UnixFileMMap(name), _mmap_offset(0), _mmap_blocksize(1<<28) {}

    bool UnixFilePartMMap::open() {
        if (_error) return false;
        if (_open) { _error = true; return false; }

        _file.reset(new OpenFile(_name.c_str(), O_RDONLY, false));
        if (_file->error) {
            std::cerr << "ERROR - Could not open '" << _name \
                      << "' - error: " << strerror(errno) << std::endl;
            _error = true;
            return false;
        }
        struct stat buffer;
        static_assert(sizeof(buffer.st_size) >= sizeof(size_t), "OS Size information is 32 bit!");
// See https://github.com/JohannesEbke/a4/issues/34
//        static_assert(sizeof(size_t) >= sizeof(uint64_t), "size_t is 32 bit!");
        if (fstat(_file->no, &buffer) == -1) {
            std::cerr << "ERROR - Could not stat '" << _name << "' - error: " << strerror(errno) << std::endl;
            return false;
        }
        _size = buffer.st_size;
        remap();
        _open = true;
        return true;
    }

    void UnixFilePartMMap::remap() {
        if (_mmap) munmap(_mmap, _mmap_blocksize);
        _mmap = ::mmap(NULL, _mmap_blocksize, PROT_READ, MAP_PRIVATE, _file->no, _mmap_offset);
        if (mmap == MAP_FAILED) {
            throw a4::Fatal("ERROR - Could not mmap '", _name, "': ", strerror(errno));
        }
    }

    bool UnixFilePartMMap::close() {
        if (_mmap) munmap(_mmap, _mmap_blocksize);
        _file.reset();
        _error = true;
        return true;
    }

    bool UnixFilePartMMap::Next(const void** data, int* size) {
        if (_error) return false;
        if (!_open && !open()) return false;
        if (_size == _position) return false;
        if (_position < _mmap_offset || _position >= _mmap_offset + _mmap_blocksize) {
            _mmap_offset = (_position/_mmap_blocksize)*_mmap_blocksize;
            remap();
        }
        *data = (uint8_t*)_mmap + _position - _mmap_offset;
        *size = _mmap_blocksize - (_position - _mmap_offset);
        if ((_size - _position) < *size) {
            *size = _size - _position;
        }
        _position += *size;
        return true;
    }

    unique<ZeroCopyStreamResource> UnixFilePartMMap::Clone(size_t offset) {
        return unique<ZeroCopyStreamResource>();
    };
        
    UnixStream::UnixStream() {};

    UnixStream::UnixStream(int file_descriptor, int block_size) {
        _impl.reset(new google::protobuf::io::FileInputStream(file_descriptor, block_size));
    };

    bool UnixStream::Next(const void** data, int* size) { return _impl->Next(data, size); }
    void UnixStream::BackUp(int count) { return _impl->BackUp(count); }
    bool UnixStream::Skip(int count) { return _impl->Skip(count); }
    google::protobuf::int64 UnixStream::ByteCount() const { return _impl->ByteCount(); }

    UnixFile::UnixFile(std::string filename, int block_size) {
        _file.reset(new OpenFile(filename.c_str(), O_RDONLY, false));
        _impl.reset(new google::protobuf::io::FileInputStream(_file->no, block_size));
    };

    class RemoteCopyingFile : public google::protobuf::io::CopyingInputStream {
        public:
            RemoteCopyingFile(std::string filename);
            ~RemoteCopyingFile();
            bool Close();
            void SetCloseOnDelete(bool value) { _close_on_delete = value; }
            int GetErrno() { return _errno; }

            // implements CopyingInputStream ---------------------------------
            int Read(void* buffer, int size);
            int Skip(int count);

            off_t seek(size_t position, int whence);
            bool Seek(size_t position) {
                _position = position; 
                return seek(position, SEEK_SET) == static_cast<int64_t>(_position);
            }
            bool SeekBack(size_t position) { 
                _position = seek(-position, SEEK_END);
                //off_t cur = seek(0, SEEK_CUR);
                //if (cur != _position) {
                //    throw a4::Fatal("Seek failed: Expected position: ", _position, " actual position: ", cur);
                //};
                return _position >= 0;
            }
            size_t Tell() const { return _position; };

        protected:
            unique< ::filesystem_calls > fscalls;
            string _filename;
            bool _is_closed;
            int _errno;
            int _fileno;
            mutable bool _previous_seek_failed;
            bool _close_on_delete;
            size_t _position;
    };

    RemoteCopyingFile::RemoteCopyingFile(std::string filename)
      : _filename(filename),
        _is_closed(false),
        _errno(0),
        _fileno(-1),
        _previous_seek_failed(false),
        _close_on_delete(true),
        _position(0) {
        if (filename.substr(0,7) == "rfio://") {
            fscalls.reset(new rfio_filesystem_calls());
        } else if (filename.substr(0,7) == "dcap://") {
            fscalls.reset(new dcap_filesystem_calls());
        } else if (filename.substr(0,7) == "hdfs://") {
            fscalls.reset(new hdfs_filesystem_calls());
            filename = filename.substr(8);
        } else {
            throw a4::Fatal("Unknown remote file type: ", filename);
        }
        if (!fscalls->loaded) throw a4::Fatal("Failure to load remote access library!");
        _fileno = fscalls->open(const_cast<char*>(filename.c_str()), O_RDONLY, 0);
        _errno = fscalls->last_errno();
        if (_errno != 0 or _fileno == -1) throw a4::Fatal("Could not open ", filename, " - Error ", _errno);
    }

    RemoteCopyingFile::~RemoteCopyingFile() {
        if (!Close()) {
          GOOGLE_LOG(ERROR) << "close() failed: " << strerror(_errno);
        }
    }

    bool RemoteCopyingFile::Close() {
      GOOGLE_CHECK(!_is_closed);

      _is_closed = true;
      int result;
      do {
        result = fscalls->close(_fileno);
      } while (result < 0 && fscalls->last_errno() == EINTR);
      if (result != 0) {
        // The docs on close() do not specify whether a file descriptor is still
        // open after close() fails with EIO.  However, the glibc source code
        // seems to indicate that it is not.
        _errno = fscalls->last_errno();
        return false;
      }

      return true;
    }

    int RemoteCopyingFile::Read(void* buffer, int size) {
        GOOGLE_CHECK(!_is_closed);

        int result;
        do {
            result = fscalls->read(_fileno, buffer, size);
        } while (result < 0 && fscalls->last_errno() == EINTR);

        if (result < 0) {
            // Read error (not EOF).
            _errno = fscalls->last_errno();
        }
        _position += size;
        return result;
    }

    int RemoteCopyingFile::Skip(int count) {
      GOOGLE_CHECK(!_is_closed);
      _position += count;
      if (!_previous_seek_failed &&
          fscalls->lseek(_fileno, count, SEEK_CUR) != (off_t)-1) {
        // Seek succeeded.
        return count;
      } else {
        // Failed to seek.

        // Note to self:  Don't seek again.  This file descriptor doesn't
        // support it.
        _previous_seek_failed = true;

        // Use the default implementation.
        return CopyingInputStream::Skip(count);
      }
    }

    off_t RemoteCopyingFile::seek(size_t position, int whence) {
        GOOGLE_CHECK(!_is_closed);
        if (!_previous_seek_failed) {
            off_t res = fscalls->lseek(_fileno, position, whence);
            if (res == (off_t)-1) _previous_seek_failed = true;
            return res;
        }
        return false;
    }
    
    RemoteFile::RemoteFile(std::string filename, int block_size) {
        _block_size = block_size;
        _byte_count = 0;    
        _filename = filename;
        _file.reset(new RemoteCopyingFile(filename));
        new_adaptor();
    };
    bool RemoteFile::Next(const void** data, int* size) { return _adaptor->Next(data, size); };
    void RemoteFile::BackUp(int count) { return _adaptor->BackUp(count); };
    bool RemoteFile::Skip(int count) { return _adaptor->Skip(count); };
    bool RemoteFile::Seek(size_t position) { _adaptor.reset(); bool res = _file->Seek(position); new_adaptor(); return res; }
    bool RemoteFile::SeekBack(size_t position) { _adaptor.reset(); bool res = _file->SeekBack(position); new_adaptor(); return res; }
    size_t RemoteFile::Tell() const { _adaptor.reset(); size_t res = _file->Tell(); _byte_count = res; new_adaptor(true); return res; }
    google::protobuf::int64 RemoteFile::ByteCount() const { return _byte_count + _adaptor->ByteCount(); };
    void RemoteFile::new_adaptor(bool tell) const {
        if(!tell) _byte_count = Tell();
        _adaptor.reset(new google::protobuf::io::CopyingInputStreamAdaptor(_file.get(), _block_size));
    }

    unique<ZeroCopyStreamResource> resource_from_url(std::string url) {
        boost::trim(url);
        if (url == "-") return unique<UnixStream>(new UnixStream(STDIN_FILENO));

        bool try_mmap = true;

        // Deal with rfio, dcap and file URLs
        string proto = url.substr(0,7);
        if (proto == "rfio://" or proto == "dcap://" or proto == "hdfs://") {
            return unique<RemoteFile>(new RemoteFile(url, default_network_block_size));
        } else if (proto == "file://") {
            url = url.substr(7);
        } else if (proto == "nomm://") {
            url = url.substr(7);
            try_mmap = false;
        }

        struct stat buffer;
        if (stat(url.c_str(), &buffer) == -1) throw a4::Fatal("Could not open ", url);

        // If it's a FIFO, use a non-seeking buffer
        if (S_ISFIFO(buffer.st_mode) || !try_mmap) return unique<UnixFile>(new UnixFile(url));
        else return unique<UnixFilePartMMap>(new UnixFilePartMMap(url));
        //else return unique<UnixFileMMap>(new UnixFileMMap(url));
    }

};};

