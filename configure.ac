AC_INIT([a4],[0.1])
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign nostdinc -Wall -Werror])

AC_PROG_LN_S

# specify language
AC_PROG_CC
AC_PROG_CXX([g++44 g++4.4 g++-4.4 g++46 g++4.6 g++-4.6 g++45 g++4.5 g++-4.5 g++ gcc cl KCC CC cxx cc++ xlC aCC c++])
AC_LANG([C++])
AC_LANG_CXX_0X
AM_PATH_PYTHON([2.6])
LT_INIT

BOOST_REQUIRE([1.33])
BOOST_PROGRAM_OPTIONS
BOOST_SYSTEM

AC_PROTOBUF_CHECK([],[AC_PROTOBUF_DOWNLOAD])

export A4_ROOT="$srcdir"
A4_INIT
A4_REQUIRE([a4io], [a4io.h])

AC_CONFIG_HEADERS([a4io/src/config.h])
AC_CONFIG_HEADERS([a4process/src/config.h])

AC_CONFIG_FILES([Makefile this_a4.sh.in:this_a4.sh.ac])
AC_CONFIG_FILES([a4io/Makefile])
AC_CONFIG_FILES([a4atlas/Makefile])
AC_CONFIG_FILES([a4process/Makefile])

CPPFLAGS+=" -I$srcdir/a4io/src -Ia4io/src "
CPPFLAGS+=" -I$srcdir/a4atlas/src -Ia4atlas/src "
CPPFLAGS+=" $A4_CPPFLAGS -I$srcdir/a4process/src -Ia4process/src "

AC_OUTPUT
