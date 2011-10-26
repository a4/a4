#include <stdlib.h>
// For mkdtemp.

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dlfcn.h>

#include <iostream>
#include <string>
using std::string;

#include <gtest/gtest.h>

#include <a4/config.h>

class filesystem_calls {
public:
    int    (*open)(const char*, int, mode_t);
    size_t (*read)(int, void*, size_t);
    off_t  (*lseek)(int, off_t, int);
    size_t (*write)(int, const void*, size_t);
    int    (*close)(int);
    int    (*get_errno)();
    int* eerrno;
};


class rfio_filesystem_calls : public filesystem_calls {
public:
    rfio_filesystem_calls() {
        _library = dlopen("libdpm.so", RTLD_LAZY);
        
        assert(!dlerror());
        assert(_library);
        
        #define LOAD(what, name) \
        { \
            *reinterpret_cast<void**>(what) = dlsym(_library, name); \
            assert(!dlerror()); \
            assert(*reinterpret_cast<void**>(what)); \
        }
        
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
    
    ~rfio_filesystem_calls() {
        dlclose(_library);
    }
    
    void* _library;

private:
    rfio_filesystem_calls(const rfio_filesystem_calls&) {}
};

TEST(a4io_rfio, try_dynamic_rfio) {
        
    rfio_filesystem_calls calls;
    
    char tmpl[] = {"rfio_test_XXXXXXX"};
    errno = 0;

    char* tempdir = mkdtemp(tmpl);
    ASSERT_FALSE(errno);
    
    string tempfile(string(tempdir) + "/tempfile");
    int fd = calls.open(const_cast<char*>(tempfile.c_str()), O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_EQ(0, *calls.eerrno);
    ASSERT_NE(-1, fd);
    
    calls.close(fd);
    ASSERT_EQ(0, *calls.eerrno);
}

/*

#ifdef HAVE_LIBDPM

#include <rfio.h>

TEST(a4io_rfio, rfio) {
    char tmpl[] = {"rfio_test_XXXXXXX"};
    errno = 0;

    char* tempdir = mkdtemp(tmpl);
    ASSERT_FALSE(errno);
    
    string tempfile(string(tempdir) + "/tempfile");
    FILE* f = fopen(const_cast<char*>(tempfile.c_str()), const_cast<char*>("w"));
    ASSERT_FALSE(f == NULL);
    
    errno = 0;
    fclose(f);
    ASSERT_EQ(0, errno);
        
    unlink(const_cast<char*>(tempfile.c_str()));
    rmdir(tempdir);
}

#endif // (HAVE_LIBDPM)
*/
