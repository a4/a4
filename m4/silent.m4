
AC_DEFUN([A4_SILENT], [

# Conditionally enable silent rules
m4_ifdef([AM_SILENT_RULES],[
    AM_SILENT_RULES([yes])
    # introduce manually the PROTOC prefix for silent rules if they are there.
    # ‘@&t@’ expands to nothing, so we can use it to prevent warnings about
    # a missing macro
A4_SET_AM_V_PROTOC='
A@&t@M_V_PROTOC = $(am__v_PROTOC_$(V))
am__v_PROTOC_ = $(am__v_PROTOC_$(AM_DEFAULT_VERBOSITY))
am__v_PROTOC_0 = @echo "  PROTOC" $$@@;'

    AC_SUBST(A4_SET_AM_V_PROTOC)
    AC_ARG_ENABLE([gccfilter], AS_HELP_STRING([--disable-gccfilter], [Disables the use of gccfilter for error messages (Use 'make V=1' to disable it dynamically)]))
    AS_IF([test "x$enable_gccfilt" != "xno"], [
        AC_SUBST([GCCFILTER_0], ["\$(top_srcdir)/common/gccfilter/gccfilter --colorize --remove-template-args --hide-with-clause --remove-namespaces -s std --remove-path "])
        AC_SUBST([GCCFILTER_], ["\$(GCCFILTER_\$(AM_DEFAULT_VERBOSITY))"])
        AC_SUBST([GCCFILTER], ["\$(GCCFILTER_\$(V))"])
    ])


],[
    AC_SUBST([A4_SET_AM_V_PROTOC],[])
])

])

