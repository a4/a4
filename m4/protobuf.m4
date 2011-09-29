#serial 1

# AC_PROTOBUF_DOWNLOAD
# ---------------------------
AC_DEFUN([AC_PROTOBUF_DOWNLOAD],[
  # find absolute paths
  case $srcdir in
   /*) abs_srcdir=$srcdir ;;
   *)  abs_srcdir=$PWD/$srcdir ;;
  esac

  if ! test -d "$srcdir/protobuf"; then
    if ! test -e "$srcdir/protobuf-2.4.1.tar.bz2"; then
      AC_MSG_WARN([Downloading google protobuf library...])
      curl -f http://protobuf.googlecode.com/files/protobuf-2.4.1.tar.bz2 > $srcdir/protobuf-2.4.1.tar.bz2
      if test $? != 0; then
        AC_MSG_ERROR([Missing google protoc compiler, download failed.])
      fi
    fi
    if ! tar -xj -C "$srcdir" -f "$srcdir/protobuf-2.4.1.tar.bz2"; then
      AC_MSG_ERROR([Could not unpack "$srcdir/protobuf-2.4.1.tar.bz2"!])
    fi
    mv "$srcdir/protobuf-2.4.1" "$srcdir/protobuf"
  fi
  AC_CONFIG_SUBDIRS([protobuf])
  AC_SUBST([PROTOBUF_BUILD], ["protobuf"])
  export PROTOBUF_PRIVATE_BUILDDIR="$PWD/protobuf/src"
  export PROTOBUF_PRIVATE_SRCDIR="$abs_srcdir/protobuf/src"
  AC_SUBST([PROTOBUF_PROTOC], ["$PROTOBUF_PRIVATE_BUILDDIR/protoc"])
  AC_SUBST([PROTOBUF_LIBS], ["-pthread -L$PROTOBUF_PRIVATE_BUILDDIR/.libs -lprotobuf -lz -lpthread"])
  AC_SUBST([PROTOBUF_CFLAGS], ["-pthread -I$PROTOBUF_PRIVATE_SRCDIR"])


])


# AC_PROTOBUF_CHECK([action-if-yes],[action-if-no])
# Find protobuf library in --with-protobuf or PROTOBUF_ROOT
# --------------------------------------
AC_DEFUN([AC_PROTOBUF_CHECK], [
  AC_ARG_WITH([protobuf], 
              [AS_HELP_STRING([--with-protobuf=DIR],
              [prefix of protobuf @<:@guess@:>@])])dnl
  AC_ARG_VAR([PROTOBUF_ROOT],[Location of Google protobuf installation])dnl
  AC_ARG_VAR([PROTOBUF_PRIVATE_BUILDDIR],[Alternative: build directory of protobuf library (no checks)])dnl
  AC_ARG_VAR([PROTOBUF_PRIVATE_SRCDIR],[Alternative: source directory of protobuf library (no checks)])dnl

  if test x"$PROTOBUF_PRIVATE_BUILDDIR" != x; then
    if test x"$PROTOBUF_PRIVATE_SRCDIR" != x; then
        AC_SUBST([PROTOBUF_PROTOC], ["$PROTOBUF_PRIVATE_BUILDDIR/protoc"])
        AC_SUBST([PROTOBUF_LIBS], ["-pthread -L$PROTOBUF_PRIVATE_BUILDDIR/.libs -lprotobuf -lz -lpthread"])
        AC_SUBST([PROTOBUF_CFLAGS], ["-pthread -I$PROTOBUF_PRIVATE_SRCDIR"])
        $1
    else
      AC_MSG_ERROR([PROTOBUF_PRIVATE_SRCDIR and PROTOBUF_PRIVATE_BUILDDIR have both to be specified if they are to be used!])
    fi
  else
    if test x"$PROTOBUF_PRIVATE_BUILDDIR" != x; then
      AC_MSG_ERROR([PROTOBUF_PRIVATE_SRCDIR and PROTOBUF_PRIVATE_BUILDDIR have both to be specified if they are to be used!])
    fi
  fi

  # If PROTOBUF_ROOT is set and the user has not provided a value to
  # --with-protobuf, then treat PROTOBUF_ROOT as if it the user supplied it.
  if test x"$PROTOBUF_ROOT" != x; then
    if test x"$with_protobuf" == x; then
      AC_MSG_NOTICE([Detected PROTOBUF_ROOT; continuing with --with-protobuf=$PROTOBUF_ROOT])
      with_protobuf=$PROTOBUF_ROOT
    else
      AC_MSG_NOTICE([Detected PROTOBUF_ROOT=$PROTOBUF_ROOT, but overridden by --with-protobuf=$with_protobuf])
    fi
  fi

  if test x"$PROTOBUF_PROTOC" == x; then
    if test x"$with_protobuf" != x; then
      echo "checking for protoc in $with_protobuf/bin"
      AC_PATH_PROG([PROTOBUF_PROTOC], [protoc], [], [$with_protobuf/bin])
    else
      echo "checking for protoc in PATH"
      AC_PATH_PROG([PROTOBUF_PROTOC], [protoc], [])
    fi
  fi
  if test x"$PROTOBUF_PROTOC" == x; then
    :
    protobuf_not_there=yes
  else
    export PKG_CONFIG_PATH=$with_protobuf
    PKG_CHECK_MODULES([PROTOBUF], [protobuf], [
      AC_SUBST([PROTOBUF_PROTOC])
      AC_SUBST([PROTOBUF_LIBS])
      AC_SUBST([PROTOBUF_CFLAGS])
      AC_SUBST([PROTOBUF_ROOT], [$with_protobuf])
    ], [protobuf_not_there=yes])
  fi
  if test x$protobuf_not_there == xyes; then
    :
    $2
  fi
])


