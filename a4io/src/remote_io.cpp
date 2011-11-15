#include "remote_io.h"

#include <dlfcn.h>
#include <iostream>
#include <assert.h>

#define LOAD(what, name) \
{ \
    *reinterpret_cast<void**>(what) = dlsym(_library, name); \
    assert(!dlerror()); \
    assert(what); \
}

rfio_filesystem_calls::rfio_filesystem_calls() 
{
    _library = dlopen("libdpm.so", RTLD_LAZY);
    
    if (dlerror()) return;
    if (!_library) return;
    
    LOAD(&_open, "rfio_open");
    LOAD(&_read, "rfio_read");
    LOAD(&_lseek, "rfio_lseek");
    LOAD(&_write, "rfio_write");
    LOAD(&_close, "rfio_close");
    
    // This function call is probably invalid
    LOAD(&_get_errno, "C__rfio_errno");
    
    eerrno = reinterpret_cast<int*>(dlsym(_library, "rfio_errno"));
    assert(eerrno);
    loaded = true;    
}

int rfio_filesystem_calls::last_errno() {
    return *eerrno;
}

rfio_filesystem_calls::~rfio_filesystem_calls() {
    if (_library) dlclose(_library);
}
    
dcap_filesystem_calls::dcap_filesystem_calls() {
    _library = dlopen("libdcap.so", RTLD_LAZY);
    
    if (dlerror()) return;
    if (!_library) return;
    
    LOAD(&_open, "dc_open");
    LOAD(&_read, "dc_read");
    LOAD(&_lseek, "dc_lseek");
    LOAD(&_write, "dc_write");
    LOAD(&_close, "dc_close");
    LOAD(&_set_debug_level, "dc_setDebugLevel");
    LOAD(&_internal_strerror, "dc_strerror");
    //std::cerr << "Initialized DCAP filesystem calls" << std::endl;
    //set_debug_level(32);
    loaded = true;    
}
int dcap_filesystem_calls::last_errno() {
    int nr = errno;
    if (nr != 0) {
        const char * err = _internal_strerror(nr);
        if (err) std::cerr << "Internal DCAP error: " << err << std::endl;
    }
    return nr;
}

dcap_filesystem_calls::~dcap_filesystem_calls() {
    if (_library) dlclose(_library);
}

hdfs_filesystem_calls::hdfs_filesystem_calls() {
    _errno = 0;
    if(!dlopen("libjvm.so", RTLD_LAZY) or dlerror()) {
        if (dlerror()) std::cerr << "libjvm.so dlopen failed: " << dlerror() << std::endl;
        else std::cerr << "libjvm.so is not available or loading failed!" << std::endl;
        return;
    };

    _library = dlopen("libhdfs.so", RTLD_LAZY);
    if(!_library or dlerror()) {
        if (dlerror()) std::cerr << "libhdfs.so dlopen failed: " << dlerror() << std::endl;
        else std::cerr << "libhdfs.so is not available or loading failed!" << std::endl;
        return; 
    }
    
    LOAD(&_hdfsConnect, "hdfsConnect");
    LOAD(&_hdfsOpenFile, "hdfsOpenFile");
    LOAD(&_hdfsCloseFile, "hdfsCloseFile");
    LOAD(&_hdfsSeek, "hdfsSeek");
    LOAD(&_hdfsTell, "hdfsTell");
    LOAD(&_hdfsRead, "hdfsRead");
    LOAD(&_hdfsWrite, "hdfsWrite");
    LOAD(&_hdfsGetPathInfo, "hdfsGetPathInfo"); 
    LOAD(&_hdfsFreeFileInfo,"hdfsFreeFileInfo");
    LOAD(&_hdfsDisconnect, "hdfsDisconnect");

    fs = _hdfsConnect("default", 0);
    if(!fs) {
        std::cerr << "Failed to connect to hdfs!" << std::endl;
        return;
    }
    loaded = true;    
}
int hdfs_filesystem_calls::last_errno() {
    return 0;
}

hdfs_filesystem_calls::~hdfs_filesystem_calls() {
    if (_library) {
        if (fs) {
            _hdfsDisconnect(fs);
        }
        dlclose(_library);
    }
}

int    hdfs_filesystem_calls::open(const char* name, int type, mode_t opts) {
    hdfsFile f = _hdfsOpenFile(fs, name, type, opts, 0, 0);
    if(f == NULL) return _errno = -1;

    hdfsFileInfo * fi = _hdfsGetPathInfo(fs, name);
    if (fi == NULL)  return _errno = -1;

    _files.push_back(f);
    _file_sizes.push_back(fi->mSize);
    _hdfsFreeFileInfo(fi, 1);
    
    return _files.size()-1;
};

int    hdfs_filesystem_calls::close(int fd) {
    return _errno = _hdfsCloseFile(fs, _files[fd]);
};

size_t hdfs_filesystem_calls::read(int fd, void* buf, size_t sz) {
    return _hdfsRead(fs, _files[fd], buf, sz);
};

off_t hdfs_filesystem_calls::lseek(int fd, off_t pos, int whence) {
    if (whence == SEEK_CUR) pos += _hdfsTell(fs, _files[fd]);
    else if (whence == SEEK_END) {
        pos = _file_sizes[fd] - pos;
    }
    int res = _hdfsSeek(fs, _files[fd], pos);
    if (res == -1) return _errno = -1;

    return _hdfsTell(fs, _files[fd]);
};

size_t hdfs_filesystem_calls::write(int fd, const void* buf, size_t sz) {
    return _hdfsWrite(fs, _files[fd], buf, sz);
};

