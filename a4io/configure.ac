AC_INIT([a4io], [0.1], [johannes.ebke@physik.uni-muenchen.de])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/test_rw.cpp)
AM_INIT_AUTOMAKE([foreign nostdinc -Wall -Werror]) #subdir-objects

# specify language
AC_PROG_CC
AC_PROG_CXX([g++46 g++4.6 g++-4.6 g++45 g++4.5 g++-4.5 g++44 g++4.4 g++-4.4 g++ gcc cl KCC CC cxx cc++ xlC aCC c++])
AC_LANG([C++])
AC_LANG_CXX_0X
AM_PATH_PYTHON([2.6])
LT_INIT

AC_PROTOBUF_CHECK([:],[AC_MSG_ERROR("Requires google protobuf library!")])

BOOST_REQUIRE([1.33])
BOOST_SMART_PTR

AC_DEFINE([shared], [boost::shared_ptr], [Define default shared_ptr class])
AC_DEFINE([static_shared_cast], [boost::static_pointer_cast], [Define static cast of shared ptr class])
AC_DEFINE([dynamic_shared_cast], [boost::dynamic_pointer_cast], [Define dynamic cast of shared ptr class])
AC_DEFINE([reinterpret_shared_cast], [boost::reinterpret_pointer_cast], [Define reinterpret cast of shared ptr class])

AH_TOP([#ifndef _CONFIG_H_])
AH_TOP([#define _CONFIG_H_])
AH_BOTTOM([#include <boost/shared_ptr.hpp>])
AH_BOTTOM([#endif // _CONFIG_H_])

AC_CONFIG_HEADERS([src/a4/a4io_config.h])
AC_CONFIG_FILES([Makefile this_a4.sh.in:this_a4.sh.ac])

CPPFLAGS+=" -I$srcdir/src -Isrc "

AC_OUTPUT
