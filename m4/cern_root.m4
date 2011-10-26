#serial 2

# A4_CERN_ROOT_SYSTEM_CHECK([action-if-yes],[action-if-no])
# Find CERN Root system  in --with-cern-root-system or ROOTSYS
# Try to find root-config otherwise
# --------------------------------------
AC_DEFUN([A4_CERN_ROOT_SYSTEM_CHECK], [
  AC_ARG_WITH([cern_root_system], 
              [AS_HELP_STRING([--with-cern-root-system=DIR],
              [prefix of the CERN Root system @<:@root-config@:>@])])dnl
  AC_ARG_VAR([ROOTSYS],[Location of the CERN Root system])dnl

  found_cern_root_system=true
  # If ROOTSYS is set and the user has not provided a value to
  # --with-cern-root-system, then treat ROOTSYS as if it the user supplied it.
  if test x"$ROOTSYS" != x; then
    if test x"$with_cern_root_system" == x; then
      AC_MSG_NOTICE([Detected ROOTSYS; continuing with --with-cern-root-system=$ROOTSYS])
      with_cern_root_system=$ROOTSYS
    else
      AC_MSG_NOTICE([Detected ROOTSYS=$ROOTSYS, but overridden by --with-cern-root-system=$with_cern_root_system])
    fi
  else
    if test x"$with_cern_root_system" != x; then
      AC_MSG_NOTICE([Using --with-cern-root-system=$with_cern_root_system])
    else
      with_cern_root_system=$(root-config --prefix)
      if test -n "$with_cern_root_system"; then
        AC_MSG_NOTICE([Using found CERN Root system at $with_cern_root_system])
      else
        AC_MSG_NOTICE([No CERN Root system specified and none found!])
        found_cern_root_system=false
      fi
    fi
  fi
  AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
           ["$DISTCHECK_CONFIGURE_FLAGS '--with-cern-root-system=$with_cern_root_system'"])dnl

  if test x"$with_cern_root_system" != x; then
    AC_MSG_NOTICE([checking for root-config in $with_cern_root_system/bin])
    AC_PATH_PROG([CERN_ROOT_SYSTEM_CONFIG], [root-config], [], [$with_cern_root_system/bin])
  else
    AC_MSG_NOTICE([Did not find CERN Root systems root-config!])
    found_cern_root_system=false
  fi
  
  if test "$found_cern_root_system" == "false"; then
    $2
  else
    AC_SUBST([CERN_ROOT_SYSTEM_LIBS], [`$CERN_ROOT_SYSTEM_CONFIG --libs`])
    AC_SUBST([CERN_ROOT_SYSTEM_GLIBS], [`$CERN_ROOT_SYSTEM_CONFIG --glibs`])
    AC_SUBST([CERN_ROOT_SYSTEM_CFLAGS], [`$CERN_ROOT_SYSTEM_CONFIG --cflags`])
    AC_SUBST([CERN_ROOT_SYSTEM_LDFLAGS], [`$CERN_ROOT_SYSTEM_CONFIG --ldflags`])
    AC_SUBST([ROOTSYS], [$with_cern_root_system])
    AC_DEFINE(HAVE_CERN_ROOT_SYSTEM, 1, ["CERN ROOT System is present"])
  fi
])

