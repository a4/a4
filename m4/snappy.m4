#serial 2

# A4_SNAPPY_CHECK([action-if-yes],[action-if-no])
# Find snappy library in --with-snappy or SNAPPY_ROOT
# Use builtin $(srcdir)/snappy if it is exists and not specified otherwise,
# Try to use system snappy if $(srcdir)/snappy does not exist
# --------------------------------------
AC_DEFUN([A4_SNAPPY_CHECK], [
  AC_ARG_WITH([snappy], 
              [AS_HELP_STRING([--with-snappy=DIR],
              [prefix of snappy @<:@builtin,system@:>@])])dnl
  AC_ARG_VAR([SNAPPY_ROOT],[Location of Google snappy installation])dnl

  # If SNAPPY_ROOT is set and the user has not provided a value to
  # --with-snappy, then treat SNAPPY_ROOT as if it the user supplied it.
  if test x"$SNAPPY_ROOT" != x; then
    if test x"$with_snappy" == x; then
      AC_MSG_NOTICE([Detected SNAPPY_ROOT; continuing with --with-snappy=$SNAPPY_ROOT])
      with_snappy=$SNAPPY_ROOT
    else
      AC_MSG_NOTICE([Detected SNAPPY_ROOT=$SNAPPY_ROOT, but overridden by --with-snappy=$with_snappy])
    fi
  else
    if test x"$with_snappy" != x; then
      AC_MSG_NOTICE([Using --with-snappy=$with_snappy])
    else
      if test -d $srcdir/snappy; then
        with_snappy=$(cd $srcdir/snappy && pwd)
        AC_MSG_NOTICE([Using builtin snappy at $with_snappy])
      else
        AC_MSG_NOTICE([No snappy specified and no builtin found, expecting snappy to be installed])
      fi
    fi
  fi
  AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
           ["$DISTCHECK_CONFIGURE_FLAGS '--with-snappy=$with_snappy'"])dnl

  if test x"$with_snappy" != x; then
    #AC_SUBST([SNAPPY_LIBS],[-lsnappy -L$with_snappy/lib])
    AC_SUBST([SNAPPY_LDFLAGS],["-L$with_snappy/lib -lsnappy"])
    AC_SUBST([SNAPPY_CPPFLAGS],[-I$with_snappy/include])
  else
      AC_CHECK_LIB(snappy, snappy_compress, 
        [
            AC_DEFINE(HAVE_SNAPPY, 1, ["Snappy is present"])
            AC_SUBST([SNAPPY_LDFLAGS],["-lsnappy"])
        ], 
        [AC_MSG_RESULT([not found])])
  fi
  
  AC_MSG_CHECKING([if snappy is available])

  a4_cppflags_save=$CPPFLAGS
  a4_ldflags_save=$LDFLAGS
  CPPFLAGS+=$SNAPPY_CPPFLAGS
  LDFLAGS+=$SNAPPY_LDFLAGS
  AC_CHECK_LIB(snappy, snappy_compress, 
    [AC_DEFINE(HAVE_SNAPPY, 1, ["Snappy is present"])], 
    [AC_MSG_RESULT([not found])],
    [-L$with_snappy/lib])
  LDFLAGS=$a4_ldflags_save
  CPPFLAGS=$a4_cppflags_save
  a4_cppflags_save=$CPPFLAGS
  CPPFLAGS+=$SNAPPY_CPPFLAGS
#    if test x"$with_snappy" != x; then
#      export PKG_CONFIG_PATH=$with_snappy/lib/pkgconfig
#    fi
#    PKG_CHECK_MODULES([SNAPPY], [snappy], [
#      AC_SUBST([SNAPPY_LIBS])
#      AC_SUBST([SNAPPY_CFLAGS])
#      AC_SUBST([SNAPPY_ROOT], [$with_snappy])
#    ], [snappy_not_there=yes])

  if test x$snappy_not_there == xyes; then
    :
    $2
  fi
])

