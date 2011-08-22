AC_INIT([a4],[0.1])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PATH_PYTHON([2.6])

# make sure to find google protobuf
AC_ARG_WITH([protobuf], [AS_HELP_STRING([--with-protobuf=/sw/my_pb], [specify local protobuf installation])], [], [with_protobuf=no])

# find absolute paths
case $srcdir in
 /*) abs_srcdir=$srcdir ;;
 *)  abs_srcdir=$PWD/$srcdir ;;
esac
case $builddir in
 /*) abs_builddir=$builddir ;;
 *)  abs_builddir=$PWD/$builddir ;;
esac
case $with_protobuf in
 /*) abs_with_protobuf=$with_protobuf ;;
 *)  abs_with_protobuf=$PWD/$with_protobuf ;;
esac

AC_SUBST([abs_with_protobuf])
AC_SUBST([pythonsitedir],[$pythondir/a4])

# find protoc compiler
if test "x$with_protobuf" != "xno"; then
    AC_PATH_PROG([PROTOC], [protoc], [none], [$abs_with_protobuf/bin])
    if test "x$PROTOC" == "xnone"; then
        AC_MSG_FAILURE([No protoc compiler found at $abs_with_protobuf/bin! Check your specified protobuf installation directory!])
    fi
else
    AC_PATH_PROG([PROTOC], [protoc], [download])
    if test "x$PROTOC" == "xdownload"; then
        if ! test -d "$srcdir/protobuf"; then
            if ! test -e "$srcdir/protobuf-2.4.1.tar.bz2"; then
                AC_MSG_WARN([Missing google protoc compiler. Trying download...])
                curl -f http://protobuf.googlecode.com/files/protobuf-2.4.1.tar.bz2 > $srcdir/protobuf-2.4.1.tar.bz2
                if test $? != 0; then
                    AC_MSG_FAILURE([Missing google protoc compiler, download failed.])
                fi
            fi
            tar -xj -C "$srcdir" -f "$srcdir/protobuf-2.4.1.tar.bz2"
            mv "$srcdir/protobuf-2.4.1" "$srcdir/protobuf"
        fi

        AC_CONFIG_SUBDIRS([protobuf])
        AC_SUBST([PROTOBUF_BUILD], ["protobuf"])
        PROTOC=["$abs_builddir/protobuf/src/protoc"]
        protobuf_LIBS=["-pthread -L$abs_builddir/protobuf/src/.libs -lprotobuf -lz -lpthread"]
        protobuf_CFLAGS=["-pthread -I$abs_srcdir/protobuf/src"]
    fi
fi

if test "x$protobuf_LIBS" == "x"; then
    # check for library and set LIBS and CFLAGS
    export PKG_CONFIG_PATH=$with_protobuf
    echo $PKG_CONFIG_PATH
    PKG_CHECK_MODULES([protobuf], [protobuf])
fi

export PROTOC="$PROTOC"
export protobuf_LIBS="$protobuf_LIBS"
export protobuf_CFLAGS="$protobuf_CFLAGS"
echo "PROTOC=$PROTOC"
echo "$protobuf_LIBS"
echo "$protobuf_CFLAGS"


AC_CONFIG_SUBDIRS([a4io])

AC_CONFIG_FILES([Makefile a4env.sh.in:a4env.sh.am])
AC_OUTPUT

