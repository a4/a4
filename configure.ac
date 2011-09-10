AC_INIT([a4],[0.1])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PATH_PYTHON([2.6])
AC_PROG_LN_S

AC_PROTOBUF_CHECK([],[AC_PROTOBUF_DOWNLOAD])

#export A4_ROOT="$ac_abs_confdir"
export A4_ROOT="../$srcdir"

AC_CONFIG_SUBDIRS([a4io])

AC_CONFIG_FILES([Makefile this_a4.sh.in:this_a4.sh.ac])
AC_OUTPUT

