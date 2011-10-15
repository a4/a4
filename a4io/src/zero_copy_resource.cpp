#include <sys/stat.h>
#include <fcntl.h>

#include "zero_copy_resource.h"

namespace a4{ namespace io{

    OpenFile::OpenFile(const char * name, int oflag) {
        error = true;
        no = size = 0;
        mmap = NULL;
        no = open(name, oflag);
        if (no < 0) {
            std::cerr << "ERROR - Could not open '" << name \
                      << "' - error: " << strerror(errno) << std::endl;
            return;
        }
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

    UnixFile::UnixFile(std::string name) : _name(name), _size(0), _position(0), _mmap(0), _error(false), _open(false) {};

    UnixFile::~UnixFile() { if (!_error) close(); };

    bool UnixFile::open() {
        if (_error) return false;
        if (_open) { _error = true; return false; }

        _file.reset(new OpenFile(_name.c_str(), O_RDONLY));
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

    bool UnixFile::close() {
        _file.reset();
        _error = true;
        return true;
    }

    bool UnixFile::Next(const void** data, int* size) {
        if (_error) return false;
        if (!_open && !open()) return false;
        if (_size == _position) return false;
        *data = (uint8_t*)_mmap + _position;
        *size = _size - _position;
        _position = _size;
        return true;
    }

    void UnixFile::BackUp(int count) {
        assert(count >= 0 && size_t(count) <= _position);
        _position -= size_t(count);
    }

    bool UnixFile::Skip(int count) {
        if (count < 0) return false;
        _position += count;
        if (_position > _size) {
            _position = _size;
            return false;
        }
        return true;            
    }

    bool UnixFile::Seek(size_t position) {
        if (_error) return false;
        if (!_open && !open()) return false;
        if (position > _size) return false;
        _position = position;
        return true;
    }

    bool UnixFile::SeekBack(size_t position) {
        if (position > _size) return false;
        return Seek(_size - position);
    };

    size_t UnixFile::Tell() const {
        return _position;
    }

    // Get a clone of this stream with the given offset
    unique<ZeroCopyStreamResource> UnixFile::Clone(size_t offset) {
        assert(_open);
        unique<UnixFile> f(new UnixFile(_name));
        f->_file = _file;
        f->_position = offset;
        f->_mmap = (uint8_t*)_mmap + offset;
        f->_error = _error;
        f->_open = _open;
        return std::move(f);
    };

    unique<ZeroCopyStreamResource> resource_from_url(std::string url) {
        /// Only UnixFile currently supported
        return unique<UnixFile>(new UnixFile(url));
    }

};};

