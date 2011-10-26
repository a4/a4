#include <stdlib.h>
// For mkdtemp.

#include <iostream>
#include <string>
using std::string;

#include <gtest/gtest.h>

#include <a4/config.h>

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

#endif // (HAVE_RFIO)
