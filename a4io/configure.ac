AC_INIT([a4io], [0.1], [johannes.ebke@physik.uni-muenchen.de])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/rwtest.cpp)
AM_INIT_AUTOMAKE([foreign nostdinc -Wall -Werror]) #subdir-objects

# specify language
AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
AM_PATH_PYTHON([2.6])

# allow specifying custom protobuf, but ignore it if the top-level configure already got it
AC_ARG_WITH([protobuf], [AS_HELP_STRING([--with-protobuf=/sw/my_pb], [specify local protobuf installation])], [], [with_protobuf=no])

# find protoc compiler
if test "x$PROTOC" == "x"; then
    if test "x$with_protobuf" != xno; then
        AC_PATH_PROG([PROTOC], [protoc], [none], [$with_protobuf/bin])
        if test "x$PROTOC" == "xnone"; then
            AC_MSG_FAILURE([No protoc compiler found at $with_protobuf/bin! Check your specified protobuf installation directory!])
        fi
    else
        AC_PATH_PROG([PROTOC], [protoc], [no])
        if test "x$PROTOC" == "xno"; then
            AC_MSG_FAILURE([Missing google protoc compiler - run configure from a4 to download it])
        fi
    fi
fi

# only do pkg-config if we do not have a parent configure do it already
if test "x$protobuf_LIBS" == "x"; then
    export PKG_CONFIG_PATH=$with_protobuf
    PKG_CHECK_MODULES([protobuf], [protobuf])
fi

AC_SUBST([PROTOC])
AC_SUBST([protobuf_LIBS])
AC_SUBST([protobuf_CFLAGS])
echo "PROTOC=$PROTOC"
echo "$protobuf_LIBS"
echo "$protobuf_CFLAGS"

# Libtool and BOOST
AC_PROG_LIBTOOL
BOOST_REQUIRE([1.40])
#BOOST_FILESYSTEM
BOOST_FOREACH
#BOOST_FORMAT
#BOOST_PROGRAM_OPTIONS
BOOST_SMART_PTR
#BOOST_THREADS

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

CPPFLAGS="$CPPFLAGS -std=c++0x -I$srcdir/src -Isrc"

AC_OUTPUT
