#serial 1

# A4_GTEST_CHECK()
# Find gtest library in --with-gtest or GTEST_ROOT
# Use builtin $(srcdir)/gtest if it is exists and not specified otherwise,
# Try to use system gtest if $(srcdir)/gtest does not exist
# --------------------------------------
AC_DEFUN([A4_GTEST_CHECK], [
  AC_ARG_WITH([gtest], 
              [AS_HELP_STRING([--with-gtest=DIR],
              [prefix of google test framework if not built in @<:@builtin,system@:>@])])dnl
  AC_ARG_VAR([GTEST_ROOT],[Location of Google gtest installation])dnl

  # If GTEST_ROOT is set and the user has not provided a value to
  # --with-gtest, then treat GTEST_ROOT as if it the user supplied it.
  builtin_gtest=no
  if test x"$GTEST_ROOT" != x; then
    if test x"$with_gtest" == x; then
      AC_MSG_NOTICE([Detected GTEST_ROOT; continuing with --with-gtest=$GTEST_ROOT])
      with_gtest=$GTEST_ROOT
    else
      AC_MSG_NOTICE([Detected GTEST_ROOT=$GTEST_ROOT, but overridden by --with-gtest=$with_gtest])
    fi
  else
    if test x"$with_gtest" != x; then
      AC_MSG_NOTICE([Using --with-gtest=$with_gtest])
    else
      if test -d $srcdir/gtest; then
        with_gtest=$(cd $srcdir/gtest && pwd)
        AC_CONFIG_SUBDIRS([gtest])
        AC_SUBST([GTEST_CPPFLAGS], ["-I\$(top_srcdir)/gtest/include -I\$(top_srcdir)/gtest -I\$(top_builddir)/gtest/include -pthread"])
        AC_SUBST([GTEST_LDFLAGS], [""])
        AC_SUBST([GTEST_LIBS], ["\$(top_builddir)/gtest/lib/libgtest.la -pthread"])
        AC_MSG_NOTICE([Using builtin gtest at $with_gtest])
        builtin_gtest=yes
      fi
    fi
  fi


  AS_IF([test x$builtin_gtest = xno],[
   AS_IF([test -n "${with_gtest}"],
     [AS_IF([test -x "${with_gtest}/scripts/gtest-config"],
        [GTEST_CONFIG="${with_gtest}/scripts/gtest-config"],
        [GTEST_CONFIG="${with_gtest}/bin/gtest-config"])
      AS_IF([test -x "${GTEST_CONFIG}"], [],
        [AC_MSG_RESULT([no])
         AC_MSG_ERROR([dnl
Unable to locate either a builtin or installed Google Test.
The specific location '${with_gtest}' was provided for a built or installed
Google Test, but no 'gtest-config' script could be found at this location.])
         ])],
     [AC_PATH_PROG([GTEST_CONFIG], [gtest-config])])
    AS_IF([test -n "${GTEST_CONFIG}"],
     [AC_SUBST([GTEST_CPPFLAGS],[`${GTEST_CONFIG} --cppflags`])
      AC_SUBST([GTEST_LDFLAGS],[`${GTEST_CONFIG} --ldflags`])
      AC_SUBST([GTEST_LIBS],[`${GTEST_CONFIG} --libs`])
      AC_DEFINE([HAVE_GTEST],[1],[Defined when Google Test is available.])],
     [AC_MSG_ERROR([dnl
Google Test was enabled, but no viable version could be found.])
     ])
    AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
             ["$DISTCHECK_CONFIGURE_FLAGS '--with-gtest=$with_gtest'"])dnl
  ])
])

