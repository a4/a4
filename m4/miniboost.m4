#serial 2

# _BOOST_SED_CPP(SED-PROGRAM, PROGRAM,
#                [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------
# Same as AC_EGREP_CPP, but leave the result in conftest.i.
#
# SED-PROGRAM is *not* overquoted, as in AC_EGREP_CPP.  It is expanded
# in double-quotes, so escape your double quotes.
#
# It could be useful to turn this into a macro which extracts the
# value of any macro.
m4_define([_BOOST_SED_CPP],
[AC_LANG_PREPROC_REQUIRE()dnl
AC_REQUIRE([AC_PROG_SED])dnl
AC_LANG_CONFTEST([AC_LANG_SOURCE([[$2]])])
AS_IF([dnl eval is necessary to expand ac_cpp.
dnl Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
dnl Beware of Windows end-of-lines, for instance if we are running
dnl some Windows programs under Wine.  In that case, boost/version.hpp
dnl is certainly using "\r\n", but the regular Unix shell will only
dnl strip `\n' with backquotes, not the `\r'.  This results in
dnl boost_cv_lib_version='1_37\r' for instance, which breaks
dnl everything else.
dnl Cannot use 'dnl' after [$4] because a trailing dnl may break AC_CACHE_CHECK
(eval "$ac_cpp conftest.$ac_ext") 2>&AS_MESSAGE_LOG_FD |
  tr -d '\r' |
  $SED -n -e "$1" >conftest.i 2>&1],
  [$3],
  [$4])
rm -rf conftest*
])# AC_EGREP_CPP

# BOOST_CHECK_VERSION([VERSION], [ACTION-IF-NOT-FOUND])
# -----------------------------------------------
# Look for Boost.  If version is given, it must either be a literal of the form
# "X.Y.Z" where X, Y and Z are integers (the ".Z" part being optional) or a
# variable "$var".
# Defines the value BOOST_CPPFLAGS.  This macro only checks for headers with
# the required version, it does not check for any of the Boost libraries.
# On # success, defines HAVE_BOOST.  On failure, calls the optional
# ACTION-IF-NOT-FOUND action if one was supplied.
# Otherwise aborts with an error message.
AC_DEFUN([BOOST_CHECK_VERSION],
[AC_REQUIRE([AC_PROG_CXX])dnl
AC_REQUIRE([AC_PROG_GREP])dnl
boost_save_IFS=$IFS
boost_version_req=$1
IFS=.
set x $boost_version_req 0 0 0
IFS=$boost_save_IFS
shift
boost_version_req=`expr "$[1]" '*' 100000 + "$[2]" '*' 100 + "$[3]"`
boost_version_req_string=$[1].$[2].$[3]
boost_save_CPPFLAGS=$CPPFLAGS
  AC_CACHE_CHECK([for Boost headers version >= $boost_version_req_string],
    [boost_cv_inc_path],
    [boost_cv_inc_path=no
AC_LANG_PUSH([C++])dnl
m4_pattern_allow([^BOOST_VERSION$])dnl
    AC_LANG_CONFTEST([AC_LANG_PROGRAM([[#include <boost/version.hpp>
#if !defined BOOST_VERSION
# error BOOST_VERSION is not defined
#elif BOOST_VERSION < $boost_version_req
# error Boost headers version < $boost_version_req
#endif
]])])
    # If the user provided a value to --with-boost, use it and only it.
    case $with_boost in #(
      ''|yes) set x '' /opt/local/include /usr/local/include /opt/include \
                 /usr/include C:/Boost/include;; #(
      *)      set x "$with_boost/include" "$with_boost";;
    esac
    shift
    for boost_dir
    do
    # Without --layout=system, Boost (or at least some versions) installs
    # itself in <prefix>/include/boost-<version>.  This inner loop helps to
    # find headers in such directories.
    #
    # Any ${boost_dir}/boost-x_xx directories are searched in reverse version
    # order followed by ${boost_dir}.  The final '.' is a sentinel for
    # searching $boost_dir" itself.  Entries are whitespace separated.
    #
    # I didn't indent this loop on purpose (to avoid over-indented code)
    boost_layout_system_search_list=`cd "$boost_dir" 2>/dev/null \
        && ls -1 | "${GREP}" '^boost-' | sort -rn -t- -k2 \
        && echo .`
    for boost_inc in $boost_layout_system_search_list
    do
      if test x"$boost_inc" != x.; then
        boost_inc="$boost_dir/$boost_inc"
      else
        boost_inc="$boost_dir" # Uses sentinel in boost_layout_system_search_list
      fi
      if test x"$boost_inc" != x; then
        # We are going to check whether the version of Boost installed
        # in $boost_inc is usable by running a compilation that
        # #includes it.  But if we pass a -I/some/path in which Boost
        # is not installed, the compiler will just skip this -I and
        # use other locations (either from CPPFLAGS, or from its list
        # of system include directories).  As a result we would use
        # header installed on the machine instead of the /some/path
        # specified by the user.  So in that precise case (trying
        # $boost_inc), make sure the version.hpp exists.
        #
        # Use test -e as there can be symlinks.
        test -e "$boost_inc/boost/version.hpp" || continue
        CPPFLAGS="$CPPFLAGS -I$boost_inc"
      fi
      AC_COMPILE_IFELSE([], [boost_cv_inc_path=yes], [boost_cv_version=no])
      if test x"$boost_cv_inc_path" = xyes; then
        if test x"$boost_inc" != x; then
          boost_cv_inc_path=$boost_inc
        fi
        break 2
      fi
    done
    done
AC_LANG_POP([C++])dnl
    ])
    case $boost_cv_inc_path in #(
      no)
        boost_errmsg="cannot find Boost headers version >= $boost_version_req_string"
        m4_if([$2], [],  [AC_MSG_ERROR([$boost_errmsg])],
                        [AC_MSG_NOTICE([$boost_errmsg])])
        $2
        ;;#(
      yes)
        BOOST_CPPFLAGS=
        ;;#(
      *)
        AC_SUBST([BOOST_CPPFLAGS], ["-I$boost_cv_inc_path"])dnl
        ;;
    esac
  if test x"$boost_cv_inc_path" != xno; then
  AC_DEFINE([HAVE_BOOST], [1],
            [Defined if the requested minimum BOOST version is satisfied])
  AC_CACHE_CHECK([for Boost's header version],
    [boost_cv_lib_version],
    [m4_pattern_allow([^BOOST_LIB_VERSION$])dnl
     _BOOST_SED_CPP([/^boost-lib-version = /{s///;s/\"//g;p;q;}],
                    [#include <boost/version.hpp>
boost-lib-version = BOOST_LIB_VERSION],
    [boost_cv_lib_version=`cat conftest.i`])])
    # e.g. "134" for 1_34_1 or "135" for 1_35
    boost_major_version=`echo "$boost_cv_lib_version" | sed 's/_//;s/_.*//'`
    case $boost_major_version in #(
      '' | *[[!0-9]]*)
        AC_MSG_ERROR([invalid value: boost_major_version=$boost_major_version])
        ;;
    esac
fi
CPPFLAGS=$boost_save_CPPFLAGS
])# BOOST_REQUIRE

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

      else
        AC_MSG_NOTICE([No boost specified and builtin miniboost not set up, expecting boost to be installed])
      fi
    fi
  fi
  AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
           ["$DISTCHECK_CONFIGURE_FLAGS '--with-boost=$with_boost'"])dnl

  BOOST_CHECK_VERSION([1.41], [$2])
])

