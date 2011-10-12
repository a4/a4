AC_PREREQ(2.59)
AC_INIT([a4],[0.1])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects -Wall -Werror])
m4_include([m4/a4.ac])
A4_REQUIRE([a4io], [a4io.h])
A4_REQUIRE([a4process], [a4process.h])
#A4_REQUIRE([a4hist], [a4hist.h])
A4_REQUIRE([a4atlas], [grl.h])

# place config.h in a4io (should be some common package)
AC_CONFIG_HEADER([a4io/src/a4/config.h])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([a4io/Makefile])
AC_CONFIG_FILES([a4process/Makefile])
#AC_CONFIG_FILES([a4hist/Makefile])
AC_CONFIG_FILES([a4atlas/Makefile])
AC_CONFIG_FILES([a4root/Makefile])

AC_CONFIG_SUBDIRS([gtest])
AC_SUBST([GTEST_CPPFLAGS], ["-I\$(top_srcdir)/gtest/include -I\$(top_srcdir)/gtest -I\$(top_builddir)/gtest/include -pthread"])
AC_SUBST([GTEST_LIBS], ["\$(top_builddir)/gtest/lib/libgtest.la -pthread"])

AC_OUTPUT
