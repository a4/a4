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
    
    LOAD(&open, "rfio_open");
    LOAD(&read, "rfio_read");
    LOAD(&lseek, "rfio_lseek");
    LOAD(&write, "rfio_write");
    LOAD(&close, "rfio_close");
    
    // This function call is probably invalid
    LOAD(&get_errno, "C__rfio_errno");
    
    eerrno = reinterpret_cast<int*>(dlsym(_library, "rfio_errno"));
    assert(eerrno);
    
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
    
    LOAD(&open, "dc_open");
    LOAD(&read, "dc_read");
    LOAD(&lseek, "dc_lseek");
    LOAD(&write, "dc_write");
    LOAD(&close, "dc_close");
    LOAD(&set_debug_level, "dc_setDebugLevel");
    LOAD(&internal_strerror, "dc_strerror");
    //set_debug_level(32);
}
int dcap_filesystem_calls::last_errno() {
    int nr = errno;
    if (nr != 0) {
        const char * err = internal_strerror(nr);
        if (err) std::cerr << "Internal DCAP error: " << err << std::endl;
    }
    return nr;
}

dcap_filesystem_calls::~dcap_filesystem_calls() {
    if (_library) dlclose(_library);
}

