#serial 1

# AC_REQUIRE_BOOST()
# Find boost library in --with-boost or BOOST_ROOT or the builtin miniboost
# --------------------------------------
AC_DEFUN([AC_REQUIRE_BOOST], [
  # find absolute paths
  case $srcdir in
   /*) abs_srcdir=$srcdir ;;
   *)  abs_srcdir=$PWD/$srcdir ;;
  esac
  AC_ARG_WITH([boost], 
              [AS_HELP_STRING([--with-boost=DIR],
              [prefix of boost @<:@builtin@:>@])])dnl
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
  fi
  if test x"$with_boost" == x; then
    if ! test -d "$srcdir/miniboost"; then
      AC_MSG_ERROR([Cannot find (mini-)boost library at $srcdir/miniboost!])
    fi
    with_boost=$abs_srcdir/miniboost
    AC_SUBST([BOOST_ROOT], [$with_boost])
    AC_SUBST([MINIBOOST_BUILD], ["miniboost"])
    AC_SUBST([BOOST_LIBS], ["-L$abs_srcdir/miniboost/stage/lib"])
    AC_SUBST([BOOST_CFLAGS], ["-I$abs_srcdir/miniboost"])
  fi
  AC_SUBST([BOOST_LIBS], ["-L$with_boost/stage/lib"])
  AC_SUBST([BOOST_CFLAGS], ["-I$with_boost"])
])

