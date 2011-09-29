AC_INIT([a4],[0.1])
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/a4.ac])

export A4_ROOT=$(cd $srcdir && pwd)
A4_INIT
A4_REQUIRE([a4io], [a4io.h])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([a4io/Makefile])
AC_CONFIG_FILES([a4atlas/Makefile])
AC_CONFIG_FILES([a4process/Makefile])

#CPPFLAGS+=" -I${top_srcdir}/a4io/src -I${top_builddir}/a4io/src "
#CPPFLAGS+=" -I${top_srcdir}/a4atlas/src -I${top_builddir}/a4atlas/src "
#CPPFLAGS+=" $A4_CPPFLAGS -I${top_srcdir}/a4process/src -I${top_builddir}/a4process/src "

AC_OUTPUT
