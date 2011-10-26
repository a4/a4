#serial 2

# A4_LIBDPM_CHECK([action-if-yes],[action-if-no])
# Find LIBDPM library in --with-LIBDPM or LIBDPM_ROOT
# Use builtin $(srcdir)/LIBDPM if it is exists and not specified otherwise,
# Try to use system LIBDPM if $(srcdir)/LIBDPM does not exist
# --------------------------------------
AC_DEFUN([A4_LIBDPM_CHECK], [
  AC_ARG_WITH([libdpm], 
              [AS_HELP_STRING([--with-libdpm=DIR],
              [prefix of libdpm @<:@builtin,system@:>@])])dnl
  AC_ARG_VAR([LIBDPM_ROOT],[Location of libdpm installation])dnl

  # If LIBDPM_ROOT is set and the user has not provided a value to
  # --with-LIBDPM, then treat LIBDPM_ROOT as if it the user supplied it.
  if test x"$LIBDPM_ROOT" != x; then
    if test x"$with_LIBDPM" == x; then
      AC_MSG_NOTICE([Detected LIBDPM_ROOT; continuing with --with-libdpm=$LIBDPM_ROOT])
      with_LIBDPM=$LIBDPM_ROOT
    else
      AC_MSG_NOTICE([Detected LIBDPM_ROOT=$LIBDPM_ROOT, but overridden by --with-libdpm=$with_LIBDPM])
    fi
  else
    if test x"$with_LIBDPM" != x; then
      AC_MSG_NOTICE([Using --with-libdpm=$with_LIBDPM])
    else
      # TODO(pwaller): somehow we need to check for /opt/lcg and /usr
      if test -d /opt/lcg; then
        with_LIBDPM=/opt/lcg
        AC_MSG_NOTICE([No libdpm specified, trying $with_LIBDPM])
      else
        with_LIBDPM=/usr
        AC_MSG_NOTICE([No libdpm specified and no builtin found, expecting LIBDPM to be installed])
      fi
    fi
  fi
  AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
           ["$DISTCHECK_CONFIGURE_FLAGS '--with-libdpm=$with_LIBDPM'"])dnl

  LIB_DIR=lib
  case $host in
  x86_64-*)
    LIB_DIR=lib64
  esac

  if test x"$with_LIBDPM" != x; then
    AC_SUBST([LIBDPM_LDFLAGS],[" -L$with_LIBDPM/$LIB_DIR -ldpm -ldl"])
    AC_SUBST([LIBDPM_CPPFLAGS],[" -I$with_LIBDPM/include/dpm"])
  fi
  
  AC_MSG_CHECKING([if libdpm is available])

  a4_cppflags_save=$CPPFLAGS
  a4_ldflags_save=$LDFLAGS
  CPPFLAGS+=$LIBDPM_CPPFLAGS
  LDFLAGS+=$LIBDPM_LDFLAGS
  
  AC_CHECK_LIB(dpm, rfio_open,
    [AC_DEFINE(HAVE_LIBDPM, 1, ["libdpm is present"])], 
    [libdpm_not_there=yes],
    [-L$with_LIBDPM/$LIB_DIR])
    
  LDFLAGS=$a4_ldflags_save
  CPPFLAGS=$a4_cppflags_save

  if test x$libdpm_not_there == xyes; then
    :
    AC_MSG_RESULT([not found])
    $2
  else
    AC_SUBST([AM_CXXFLAGS],["$AM_CXXFLAGS $LIBDPM_CPPFLAGS"])
    AC_SUBST([AM_CPPFLAGS],["$AM_CPPFLAGS $LIBDPM_CPPFLAGS"])
    AC_SUBST([AM_LDFLAGS], ["$AM_LDFLAGS  $LIBDPM_LDFLAGS"])
  fi
])

