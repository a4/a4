AC_INIT([a4io], [0.1], [johannes.ebke@physik.uni-muenchen.de])
AC_CONFIG_SRCDIR(src/reader.cpp)
AC_PREFIX_DEFAULT()
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CXX

AC_CHECK_PROG([HAS_PROTOC], [protoc], [yes], [no])
if test "x$HAS_PROTOC" = xno; then
    AC_MSG_ERROR([missing google protoc compiler. Use configure from the top-level a4 directory!])
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
