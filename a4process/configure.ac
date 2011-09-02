AC_INIT([a4process], [0.1], [johannes.ebke@physik.uni-muenchen.de])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/test_app.cpp)
AM_INIT_AUTOMAKE([foreign nostdinc -Wall -Werror]) #subdir-objects

# specify language
AC_PROG_CC
AC_PROG_CXX([g++46 g++4.6 g++-4.6 g++45 g++4.5 g++-4.5 g++44 g++4.4 g++-4.4 g++ gcc cl KCC CC cxx cc++ xlC aCC c++])
AC_LANG([C++])
AC_LANG_CXX_0X
AM_PATH_PYTHON([2.6])

LT_INIT

#AC_PROTOBUF_CHECK([:],[AC_MSG_ERROR("Requires google protobuf library!")])

BOOST_REQUIRE([1.33])
BOOST_SMART_PTR
BOOST_PROGRAM_OPTIONS

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

CPPFLAGS+=" -I$srcdir/src -Isrc "

AC_OUTPUT
