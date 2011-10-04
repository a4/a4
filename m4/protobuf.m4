#serial 2

# A4_PROTOBUF_CHECK([action-if-yes],[action-if-no])
# Find protobuf library in --with-protobuf or PROTOBUF_ROOT
# Use builtin $(srcdir)/protobuf if it is exists and not specified otherwise,
# Try to use system protobuf if $(srcdir)/protobuf does not exist
# --------------------------------------
AC_DEFUN([A4_PROTOBUF_CHECK], [
  AC_ARG_WITH([protobuf], 
              [AS_HELP_STRING([--with-protobuf=DIR],
              [prefix of protobuf @<:@builtin,system@:>@])])dnl
  AC_ARG_VAR([PROTOBUF_ROOT],[Location of Google protobuf installation])dnl

  # If PROTOBUF_ROOT is set and the user has not provided a value to
  # --with-protobuf, then treat PROTOBUF_ROOT as if it the user supplied it.
  if test x"$PROTOBUF_ROOT" != x; then
    if test x"$with_protobuf" == x; then
      AC_MSG_NOTICE([Detected PROTOBUF_ROOT; continuing with --with-protobuf=$PROTOBUF_ROOT])
      with_protobuf=$PROTOBUF_ROOT
    else
      AC_MSG_NOTICE([Detected PROTOBUF_ROOT=$PROTOBUF_ROOT, but overridden by --with-protobuf=$with_protobuf])
    fi
  else
    if test x"$with_protobuf" != x; then
      AC_MSG_NOTICE([Using --with-protobuf=$with_protobuf])
    else
      if test -d $srcdir/protobuf; then
        with_protobuf=$(cd $srcdir/protobuf && pwd)
        AC_MSG_NOTICE([Using builtin protobuf at $with_protobuf])
      else
        AC_MSG_NOTICE([No protobuf specified and no builtin found, expecting protobuf to be installed])
      fi
    fi
  fi
  AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
           ["$DISTCHECK_CONFIGURE_FLAGS '--with-protobuf=$with_protobuf'"])dnl

  if test x"$PROTOBUF_PROTOC" == x; then
    if test x"$with_protobuf" != x; then
      AC_MSG_NOTICE([checking for protoc in $with_protobuf/bin])
      AC_PATH_PROG([PROTOBUF_PROTOC], [protoc], [], [$with_protobuf/bin])
    else
      AC_MSG_NOTICE([checking for protoc in PATH])
      AC_PATH_PROG([PROTOBUF_PROTOC], [protoc], [])
    fi
  fi
  if test x"$PROTOBUF_PROTOC" == x; then
    :
    protobuf_not_there=yes
  else
    if test x"$with_protobuf" != x; then
      export PKG_CONFIG_PATH=$with_protobuf/lib/pkgconfig
    fi
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

