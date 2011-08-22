AC_INIT([a4io], [0.1], [johannes.ebke@physik.uni-muenchen.de])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/reader.cpp)
AM_INIT_AUTOMAKE([foreign -Wall -Werror]) #subdir-objects

# specify language
AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])

# use libtool
LT_INIT

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

if test "x$protobuf_LIBS" == "x"; then
    # only do pkg-config if we do not have a parent configure do it already
    export PKG_CONFIG_PATH=$with_protobuf
    PKG_CHECK_MODULES([protobuf], [protobuf])
fi

AC_SUBST([PROTOC])
AC_SUBST([protobuf_LIBS])
AC_SUBST([protobuf_CFLAGS])
echo "PROTOC=$PROTOC"
echo "$protobuf_LIBS"
echo "$protobuf_CFLAGS"

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
