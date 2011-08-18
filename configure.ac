AC_INIT([a4],[0.1])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC

# make sure to find google protobuf
AC_CHECK_PROG([HAS_PROTOC], [protoc], [yes], [no])
if test "x$HAS_PROTOC" = xno; then
    if ! test -d "$srcdir/protobuf-2.4.1"; then
        AC_MSG_WARN([Missing google protoc compiler. Try to download google protobuf...])
        curl -f http://protobuf.googlecode.com/files/protobuf-2.4.1.tar.bz2 > $srcdir/protobuf-2.4.1.tar.bz2
        if test $? != 0; then
            AC_MSG_ERROR([Missing google protoc compiler. Failed to Use configure from the top-level a4 directory!])
        fi
        tar -xjv -C "$srcdir" -f "$srcdir/protobuf-2.4.1.tar.bz2"        
    fi
else
    echo "PROTOC FOUND"
fi

# recursively configure
if test -d "$srcdir/protobuf-2.4.1"; then
    AC_CONFIG_SUBDIRS([protobuf-2.4.1])
    PROTOBUF_BUILD=protobuf-2.4.1
else
    PROTOBUF_BUILD=
fi
AC_SUBST([PROTOBUF_BUILD])
AC_CONFIG_SUBDIRS([a4io])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
