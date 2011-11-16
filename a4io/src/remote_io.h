#include <stdlib.h>
#include <inttypes.h>
#include <vector>

class filesystem_calls {
public:
    virtual ~filesystem_calls() {};
    virtual int last_errno() = 0;
    virtual int    open(const char*, int, mode_t) = 0;
    virtual size_t read(int, void*, size_t) = 0;
    virtual off_t  lseek(int, off_t, int) = 0;
    virtual size_t write(int, const void*, size_t) = 0;
    virtual int    close(int) = 0;
    bool loaded;
protected:
    int    (*_open)(const char*, int, mode_t);
    size_t (*_read)(int, void*, size_t);
    off_t  (*_lseek)(int, off_t, int);
    size_t (*_write)(int, const void*, size_t);
    int    (*_close)(int);
    int    (*_get_errno)();
    void (*_set_debug_level)(int);
    const char * (*_internal_strerror)(int);
    int* eerrno;
    filesystem_calls() : loaded(false), _open(NULL), _read(NULL), _lseek(NULL), _write(NULL),
                              _close(NULL), _get_errno(NULL), eerrno(NULL) {};
};


class rfio_filesystem_calls : public filesystem_calls {
public:
    rfio_filesystem_calls(); 
    ~rfio_filesystem_calls();
    int last_errno();
    virtual int    open(const char* p1, int p2, mode_t p3) { return _open(p1, p2, p3); };
    virtual size_t read(int p1, void* p2, size_t p3) { return _read(p1, p2, p3); };
    virtual off_t  lseek(int p1, off_t p2, int p3) { return _lseek(p1, p2, p3); };
    virtual size_t write(int p1, const void* p2, size_t p3) { return _write(p1, p2, p3); };
    virtual int    close(int p1) { return _close(p1); };
    void* _library;
private:
    rfio_filesystem_calls(const rfio_filesystem_calls&) {}
};

class dcap_filesystem_calls : public filesystem_calls {
public:
    dcap_filesystem_calls();
    ~dcap_filesystem_calls();
    int last_errno();
    virtual int    open(const char* p1, int p2, mode_t p3) { return _open(p1, p2, p3); };
    virtual size_t read(int p1, void* p2, size_t p3) { return _read(p1, p2, p3); };
    /// this fix for SEEK_END is necessary for dcap 1.2.47 (and maybe later)
    /// dcap 1.2.47 does not honor SEEK_END, but returns the right offset, so we use that.
    virtual off_t  lseek(int p1, off_t p2, int p3) { if(p3 == SEEK_END) return _lseek(p1, _lseek(p1, p2, p3), SEEK_SET); else return _lseek(p1, p2, p3); };
    virtual size_t write(int p1, const void* p2, size_t p3) { return _write(p1, p2, p3); };
    virtual int    close(int p1) { return _close(p1); };
    void* _library;
private:
    dcap_filesystem_calls(const dcap_filesystem_calls&) {}
};

typedef void* hdfsFS;
typedef void* hdfsFile;

extern "C" {
    typedef int32_t   tSize; /// size of data for read/write io ops 
    typedef time_t    tTime; /// time type in seconds
    typedef int64_t   tOffset;/// offset within the file
    typedef uint16_t  tPort; /// port
    typedef enum tObjectKind {
        kObjectKindFile = 'F',
        kObjectKindDirectory = 'D',
    } tObjectKind;

    typedef struct  {
        tObjectKind mKind;   /* file or directory */
        char *mName;         /* the name of the file */
        tTime mLastMod;      /* the last modification time for the file in seconds */
        tOffset mSize;       /* the size of the file in bytes */
        short mReplication;    /* the count of replicas */
        tOffset mBlockSize;  /* the block size for the file */
        char *mOwner;        /* the owner of the file */
        char *mGroup;        /* the group associated with the file */
        short mPermissions;  /* the permissions associated with the file */
        tTime mLastAccess;    /* the last access time for the file in seconds */
    } hdfsFileInfo;
}


class hdfs_filesystem_calls : public filesystem_calls {
public:
    hdfs_filesystem_calls();
    ~hdfs_filesystem_calls();
    int last_errno();
    virtual int    open(const char*, int, mode_t);
    virtual size_t read(int, void*, size_t);
    virtual off_t  lseek(int, off_t, int);
    virtual size_t write(int, const void*, size_t);
    virtual int    close(int);
    void* _library;
private:
    hdfsFile (*_hdfsOpenFile)(hdfsFS, const char*, int, int, short, int32_t);
    int (*_hdfsCloseFile)(hdfsFS, hdfsFile);
    int (*_hdfsSeek)(hdfsFS, hdfsFile, int64_t);
    int64_t (*_hdfsTell)(hdfsFS, hdfsFile);
    int32_t (*_hdfsRead)(hdfsFS, hdfsFile, void*, int32_t);
    int32_t (*_hdfsWrite)(hdfsFS, hdfsFile, const void*, int32_t);
    hdfsFS (*_hdfsConnect)(const char*, uint16_t);
    int (*_hdfsDisconnect)(void *);
    hdfsFileInfo * (*_hdfsGetPathInfo)(hdfsFS, const char*);
    void (*_hdfsFreeFileInfo)(hdfsFileInfo *, int);
    hdfs_filesystem_calls(const hdfs_filesystem_calls&) {}
    hdfsFS fs;
    std::vector<hdfsFile> _files;
    std::vector<off_t> _file_sizes;
    int _errno;
};
