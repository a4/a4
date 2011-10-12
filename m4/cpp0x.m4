
# AC_COMPILE_STDCXX_OX
AC_DEFUN([AC_COMPILE_STDCXX_0X], [
  AC_CACHE_CHECK(if g++ supports C++0x features without additional flags,
  ac_cv_cxx_compile_cxx0x_native,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type c;
    check_type&& cr = check_type();],,
  ac_cv_cxx_compile_cxx0x_native=yes, ac_cv_cxx_compile_cxx0x_native=no)
  AC_LANG_RESTORE
  ])

  AC_CACHE_CHECK(if g++ supports C++0x features with -std=c++0x,
  ac_cv_cxx_compile_cxx0x_cxx,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=c++0x"
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type c;
    check_type&& cr = check_type();],,
  ac_cv_cxx_compile_cxx0x_cxx=yes, ac_cv_cxx_compile_cxx0x_cxx=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])

  AC_CACHE_CHECK(if g++ supports C++0x features with -std=gnu++0x,
  ac_cv_cxx_compile_cxx0x_gxx,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type c;
    check_type&& cr = check_type();],,
  ac_cv_cxx_compile_cxx0x_gxx=yes, ac_cv_cxx_compile_cxx0x_gxx=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])

  if test "$ac_cv_cxx_compile_cxx0x_native" = yes ||
     test "$ac_cv_cxx_compile_cxx0x_cxx" = yes ||
     test "$ac_cv_cxx_compile_cxx0x_gxx" = yes; then
    AC_DEFINE(HAVE_STDCXX_0X,,[Define if g++ supports C++0x features. ])
  fi
])

# Check for library coverage of the C++0xstandard.

# AC_HEADER_STDCXX_0X
AC_DEFUN([AC_HEADER_STDCXX_0X], [
  AC_CACHE_CHECK(for ISO C++ 0x include files,
  ac_cv_cxx_stdcxx_0x,
  [AC_REQUIRE([AC_COMPILE_STDCXX_0X])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"

  AC_TRY_COMPILE([
    #include <cassert>
    #include <ccomplex>
    #include <cctype>
    #include <cerrno>
    #include <cfenv>
    #include <cfloat>
    #include <cinttypes>
    #include <ciso646>
    #include <climits>
    #include <clocale>
    #include <cmath>
    #include <csetjmp>
    #include <csignal>
    #include <cstdarg>
    #include <cstdbool>
    #include <cstddef>
    #include <cstdint>
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
    #include <ctgmath>
    #include <ctime>
    #include <cwchar>
    #include <cwctype>

    #include <algorithm>
    #include <array>
    #include <bitset>
    #include <complex>
    #include <deque>
    #include <exception>
    #include <fstream>
    #include <functional>
    #include <iomanip>
    #include <ios>
    #include <iosfwd>
    #include <iostream>
    #include <istream>
    #include <iterator>
    #include <limits>
    #include <list>
    #include <locale>
    #include <map>
    #include <memory>
    #include <new>
    #include <numeric>
    #include <ostream>
    #include <queue>
    #include <random>
    #include <regex>
    #include <set>
    #include <sstream>
    #include <stack>
    #include <stdexcept>
    #include <streambuf>
    #include <string>
    #include <tuple>
    #include <typeinfo>
    #include <type_traits>
    #include <unordered_map>
    #include <unordered_set>
    #include <utility>
    #include <valarray>
    #include <vector>
  ],,
  ac_cv_cxx_stdcxx_0x=yes, ac_cv_cxx_stdcxx_0x=no)
  AC_LANG_RESTORE
  CXXFLAGS="$ac_save_CXXFLAGS"
  ])
  if test "$ac_cv_cxx_stdcxx_0x" = yes; then
    AC_DEFINE(STDCXX_0X_HEADERS,,[Define if ISO C++ 0x header files are present. ])
  fi
])

# AC_HEADER_UNORDERED_MAP
AC_DEFUN([AC_HEADER_UNORDERED_MAP], [
  AC_CACHE_CHECK(for unordered_map,
  ac_cv_cxx_unordered_map,
  [AC_REQUIRE([AC_COMPILE_STDCXX_0X])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"
  AC_TRY_COMPILE([#include <unordered_map>], [using std::unordered_map;],
  ac_cv_cxx_unordered_map=yes, ac_cv_cxx_unordered_map=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])
  if test "$ac_cv_cxx_unordered_map" = yes; then
    AC_DEFINE(HAVE_UNORDERED_MAP,,[Define if unordered_map is present. ])
  fi
])

# AC_HEADER_UNORDERED_SET
AC_DEFUN([AC_HEADER_UNORDERED_SET], [
  AC_CACHE_CHECK(for unordered_set,
  ac_cv_cxx_unordered_set,
  [AC_REQUIRE([AC_COMPILE_STDCXX_0X])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"
  AC_TRY_COMPILE([#include <unordered_set>], [using std::unordered_set;],
  ac_cv_cxx_unordered_set=yes, ac_cv_cxx_unordered_set=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])
  if test "$ac_cv_cxx_unordered_set" = yes; then
    AC_DEFINE(HAVE_UNORDERED_SET,,[Define if unordered_set is present. ])
  fi
])

AC_DEFUN([_CPP0X_SMART_PTR_IN], [
    AC_LANG(C++)
    AC_MSG_CHECKING([for smart pointers in $1])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
        [[$3]]
        [[$2::shared_ptr<int> have_shared_ptr; $4<int> have_unique_ptr; using $2::static_pointer_cast; using $2::dynamic_pointer_cast;]])
    ], [
        AC_MSG_RESULT([yes])
        AC_DEFINE_UNQUOTED([HAVE_$1_SMART_PTR], 1, [Define to 1 if smart pointers are in $1])
    ], [
        AC_MSG_RESULT([no])
        AC_DEFINE_UNQUOTED([HAVE_$1_SMART_PTR], 0, [Define to 1 if smart pointers are in $1])
    ])
])

AC_DEFUN([AC_NAMESPACE_SHARED_PTR], [
    _CPP0X_SMART_PTR_IN([STD], [std], [#include <memory>],[std::unique_ptr])
    _CPP0X_SMART_PTR_IN([STD_TR1], [std::tr1], [#include <tr1/memory>], [std::tr1::unique_ptr])
    _CPP0X_SMART_PTR_IN([BOOST], [std::tr1], [
#include <boost/tr1/memory.hpp>
#include <boost/scoped_ptr.hpp>
    ], [boost::scoped_ptr])
])


AC_DEFUN([AC_LANG_CXX_0X], [
  AC_COMPILE_STDCXX_0X
  AC_HEADER_STDCXX_0X
  if test "x$ac_cv_cxx_compile_cxx0x_native" == "xyes"; then
      AC_SUBST([AM_CXXFLAGS],["$AM_CXXFLAGS"])
      AC_SUBST([AM_CPPFLAGS],["$AM_CPPFLAGS"])
  elif test "x$ac_cv_cxx_compile_cxx0x_cxx" == "xyes"; then
      AC_SUBST([AM_CXXFLAGS],["$AM_CXXFLAGS -std=c++0x "])
      AC_SUBST([AM_CPPFLAGS],["$AM_CPPFLAGS"])
      CXXFLAGS="$CXXFLAGS -std=c++0x"
  elif test "x$ac_cv_cxx_compile_cxx0x_gxx" == "xyes"; then
      AC_SUBST([AM_CXXFLAGS],["$AM_CXXFLAGS -std=gnu++0x "])
      AC_SUBST([AM_CPPFLAGS],["$AM_CPPFLAGS"])
      CXXFLAGS="$CXXFLAGS -std=gnu++0x"
  else
      AC_MSG_FAILURE([Compiler does not support C++0x. A4 requires g++ 4.4 or later.])
  fi
])
