#include "remote_io.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dlfcn.h>

#include <iostream>
#include <string>
using std::string;

#include <gtest/gtest.h>

#include <a4/config.h>

TEST(a4io_netio, try_dynamic_rfio) {
        
    rfio_filesystem_calls calls;
    if (!calls.loaded) return;
    
    char tmpl[] = {"rfio_test_XXXXXXX"};
    errno = 0;

    char* tempdir = mkdtemp(tmpl);
    ASSERT_FALSE(errno);
    
    string tempfile(string(tempdir) + "/tempfile");
    int fd = calls.open(const_cast<char*>(tempfile.c_str()), O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_EQ(0, calls.last_errno());
    ASSERT_NE(-1, fd);
    
    calls.close(fd);
    ASSERT_EQ(0, calls.last_errno());
}

TEST(a4io_netio, try_dynamic_dcache_io) {
        
    dcap_filesystem_calls calls;
    if (!calls.loaded) return;
    
    char tmpl[] = {"dcap_test_XXXXXXX"};
    errno = 0;

    ASSERT_EQ(0, calls.last_errno());
    string testfile = "dcap://lcg-lrz-dcache.grid.lrz-muenchen.de:22125//pnfs/lrz-muenchen.de/data/atlas/dq2/atlaslocalgroupdisk/mc11_7TeV/NTUP_SMWZ/e835_s1299_s1300_r2730_r2700_p716/mc11_7TeV.107650.AlpgenJimmyZeeNp0_pt20.merge.NTUP_SMWZ.e835_s1299_s1300_r2730_r2700_p716_tid525375_00/NTUP_SMWZ.525375._000302.root.1";

    int fd = calls.open(const_cast<char*>(testfile.c_str()), O_RDONLY, 0);
    ASSERT_EQ(0, calls.last_errno());
    ASSERT_NE(-1, fd);

    char buffer[10];
    calls.read(fd, buffer, 10);
    ASSERT_EQ(0, calls.last_errno());
    calls.close(fd);
    ASSERT_EQ(0, calls.last_errno());

    /*
    string tempfile = "dcap://lcg-lrz-dcache.grid.lrz-muenchen.de:22125//pnfs/lrz-muenchen.de/data/atlas/dq2/test_deleteme";
    fd = calls.open(const_cast<char*>(tempfile.c_str()), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_EQ(0, calls.last_errno());
    calls.write(fd, "test", 5);
    ASSERT_EQ(0, calls.last_errno());
    calls.close(fd);
    ASSERT_EQ(0, calls.last_errno());
    */
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
