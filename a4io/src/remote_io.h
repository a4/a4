#include <stdlib.h>

class filesystem_calls {
public:
    virtual ~filesystem_calls() {};
    virtual int last_errno() = 0;

    int    (*open)(const char*, int, mode_t);
    size_t (*read)(int, void*, size_t);
    off_t  (*lseek)(int, off_t, int);
    size_t (*write)(int, const void*, size_t);
    int    (*close)(int);
protected:
    int    (*get_errno)();
    void (*set_debug_level)(int);
    const char * (*internal_strerror)(int);
    int* eerrno;
    filesystem_calls() : open(NULL), read(NULL), lseek(NULL), write(NULL),
                              close(NULL), get_errno(NULL), eerrno(NULL) {};
};


class rfio_filesystem_calls : public filesystem_calls {
public:
    rfio_filesystem_calls(); 
    ~rfio_filesystem_calls();
    int last_errno();
    void* _library;
private:
    rfio_filesystem_calls(const rfio_filesystem_calls&) {}
};

class dcap_filesystem_calls : public filesystem_calls {
public:
    dcap_filesystem_calls();
    ~dcap_filesystem_calls();
    int last_errno();
    void* _library;
private:
    dcap_filesystem_calls(const dcap_filesystem_calls&) {}
};

