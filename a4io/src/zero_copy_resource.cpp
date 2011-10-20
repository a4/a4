#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>

#include "zero_copy_resource.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace a4{ namespace io{

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
        if (!do_mmap) return;
        struct stat buffer;
        static_assert(sizeof(buffer.st_size) >= sizeof(size_t), "OS Size information is 32 bit!");
        static_assert(sizeof(size_t) >= sizeof(uint64_t), "size_t is 32 bit!");
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

    unique<ZeroCopyStreamResource> resource_from_url(std::string url) {
        if (url == "-") return unique<UnixStream>(new UnixStream(STDIN_FILENO));
        struct stat buffer;
        if (stat(url.c_str(), &buffer) == -1) throw a4::Fatal("Could not open ", url);

        // If it's a FIFO, use a non-seeking bufferunique<UnixFile>(new UnixFile(url));
        if (S_ISFIFO(buffer.st_mode)) return unique<UnixFile>(new UnixFile(url));
        else return unique<UnixFileMMap>(new UnixFileMMap(url));
    }

};};

