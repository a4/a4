AC_INIT([a4process], [0.1], [johannes.ebke@physik.uni-muenchen.de])
m4_include([m4/a4.ac])
AC_CONFIG_SRCDIR(src/test_app.cpp)

BOOST_REQUIRE([1.33])
BOOST_PROGRAM_OPTIONS
BOOST_SYSTEM

A4_INIT
A4_REQUIRE([a4io], [a4io.h])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile])
CPPFLAGS+=" $A4_CPPFLAGS "

AC_OUTPUT
