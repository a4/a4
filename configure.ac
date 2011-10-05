AC_INIT([a4],[0.1])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_LIBOBJ_DIR([portable])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign silent-rules subdir-objects -Wall -Werror])

m4_include([m4/a4.ac])


A4_INIT
A4_REQUIRE([a4io], [a4io.h])
A4_REQUIRE([a4process], [a4process.h])
#A4_REQUIRE([a4hist], [a4hist.h])
A4_REQUIRE([a4atlas], [grl.h])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([a4io/Makefile])
AC_CONFIG_FILES([a4process/Makefile])
#AC_CONFIG_FILES([a4hist/Makefile])
AC_CONFIG_FILES([a4atlas/Makefile])

AC_OUTPUT
