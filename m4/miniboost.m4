#serial 2


# A4_BOOST_CHECK([action-if-yes],[action-if-no])
# Find miniboost library in --with-boost or BOOST_ROOT
# Use builtin $(srcdir)/miniboost if it is exists and not specified otherwise,
# Try to use system boost if $(srcdir)/miniboost does not exist
# --------------------------------------
AC_DEFUN([A4_BOOST_CHECK], [
  AC_ARG_WITH([boost], 
              [AS_HELP_STRING([--with-boost=DIR],
              [prefix of boost @<:@builtin,system@:>@])])dnl
  AC_ARG_VAR([BOOST_ROOT],[Location of Boost installation])dnl

  # If BOOST_ROOT is set and the user has not provided a value to
  # --with-boost, then treat BOOST_ROOT as if it the user supplied it.
  if test x"$BOOST_ROOT" != x; then
    if test x"$with_boost" == x; then
      AC_MSG_NOTICE([Detected BOOST_ROOT; continuing with --with-boost=$BOOST_ROOT])
      with_boost=$BOOST_ROOT
    else
      AC_MSG_NOTICE([Detected BOOST_ROOT=$BOOST_ROOT, but overridden by --with-boost=$with_boost])
    fi
  else
    if test x"$with_boost" != x; then
      AC_MSG_NOTICE([Using --with-boost=$with_boost])
    else
      if test -d $srcdir/miniboost; then
        with_boost=$(cd $srcdir/miniboost && pwd)
        AC_MSG_NOTICE([Using builtin miniboost at $with_boost])
        AC_SUBST([BOOST_CPPFLAGS], ["-I$with_boost/include"])
        AC_SUBST([BOOST_LDPATH], ["$with_boost"])
        AC_SUBST([BOOST_LIBS], ["-L$with_boost/lib -lboost_thread -lboost_program_options -pthread"])
      else
        AC_MSG_NOTICE([No boost specified and builtin miniboost not set up, expecting boost to be installed])
      fi
    fi
  fi
  AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
           ["$DISTCHECK_CONFIGURE_FLAGS '--with-boost=$with_boost'"])dnl
  AC_SUBST([BOOST_ROOT], [$with_boost])

  BOOST_STATIC
  BOOST_REQUIRE([1.41], [$2])

  # Find boost in strange locations
  if test x"$BOOST_LDPATH" == x; then
    BOOST_PROGRAM_OPTIONS
    BOOST_THREADS
    AC_SUBST([BOOST_LIBS], ["$BOOST_PROGRAM_OPTIONS_LDFLAGS $BOOST_PROGRAM_OPTIONS_LIBS $BOOST_THREAD_LIBS"])
  fi

])

