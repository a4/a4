# A4_INIT()
# -----------------------------------------------
# Set up --with-a4
AC_DEFUN([A4_INIT], [
AC_ARG_WITH([a4],
   [AS_HELP_STRING([--with-a4=DIR],
                   [prefix of A4 @<:@guess@:>@])])dnl
AC_ARG_VAR([A4_ROOT],[Location of A4 installation or source directory])dnl

# If A4_ROOT is set and the user has not provided a value to
# --with-a4, then treat A4_ROOT as if it the user supplied it.
if test x"$A4_ROOT" != x; then
  if test x"$with_a4" = x; then
    AC_MSG_NOTICE([Detected A4_ROOT; continuing with --with-a4=$A4_ROOT])
    with_a4=$A4_ROOT
  else
    AC_MSG_NOTICE([Detected A4_ROOT=$A4_ROOT, but overridden by --with-a4=$with_a4])
  fi
fi

AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
         ["$DISTCHECK_CONFIGURE_FLAGS '--with-a4=$with_a4'"])dnl

A4_CPPFLAGS=""
A4_LIBS=""
with_a4_is_setup=yes
])# A4_REQUIRE

# A4_REQUIRE([PACKAGE], [HEADERFILE])
# -----------------------------------------------
# Look for the given A4 package, containing HEADERFILE
# Appends to A4_CPPFLAGS and A4_LIBS 
# On failure, calls the optional
# ACTION-IF-NOT-FOUND action if one was supplied.
# Otherwise aborts with an error message.
AC_DEFUN([A4_REQUIRE], [
    if test x$with_a4_is_setup == x; then
        AC_MSG_ERROR([Put WITH_A4 before any A4\_REQUIRE macros!]);
    fi
    AC_LANG_ASSERT([C++])
    if test x"$with_a4" != x; then
        if ! test -d $with_a4/$1; then
            AC_MSG_ERROR([Directory $with_a4 does not contain package $1!])
        else 
            if test -d "$with_a4/$1/include"; then
                A4_CPPFLAGS+=" -I$with_a4/$1/include "
                A4_LIBS+=" -L$with_a4/$1/libs -l$1 "
            else
                A4_CPPFLAGS+=" -I$with_a4/$1 -I$with_a4/$1/src "
                A4_LIBS+=" -L$builddir/$1/.libs -L$builddir/$1/src/.libs -l$1 "
            fi
        fi
    fi
    a4_cppflags_save=$CPPFLAGS
    CPPFLAGS+=$A4_CPPFLAGS
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <a4/$2>]])],
        [:],
        [AC_MSG_FAILURE([Did not find header files for $1!])])
    # We cannot check the linker since the other pack might not be compiled yet
    CPPFLAGS=$a4_cppflags_save
])# A4_REQUIRE_PACKAGE
