ACLOCAL_AMFLAGS = -I m4

A4PACK=root

include m4/a4.am
include protobuf.am

# Configuration options
AM_CPPFLAGS+=$(A4_A4IO_CPPFLAGS) $(A4_A4PROCESS_CPPFLAGS) $(A4_A4HIST_CPPFLAGS) $(CERN_ROOT_SYSTEM_CFLAGS) $(PROTOBUF_CFLAGS) $(SNAPPY_CPPFLAGS) $(BOOST_CPPFLAGS)
LDADD=$(A4_A4IO_LIBS) $(A4_A4PROCESS_LIBS) $(A4_A4HIST_LIBS) $(CERN_ROOT_SYSTEM_LIBS) $(PROTOBUF_LIBS) $(SNAPPY_LDFLAGS) $(BOOST_LIBS)

bin_PROGRAMS=a4results2root root2a4
a4results2root_SOURCES = src/a4results2root.cpp

root2a4_SOURCES = src/root2a4.cpp
nodist_root2a4_SOURCES = $(PROTOBUF_CC) $(PROTOBUF_H)

EXTRA_DIST += src/convertable_types.cc

## Test programs
check_PROGRAMS =

# TODO(pwaller): Make use of EXTRA_xyz_SOURCES= in order to conditinally build A4ROOT.

# """
# By the way, I also want to make ROOT only a dependency for a4root
# as a quick hack-around I made it a dependency for A4.
# Do you know how to optionally build sources depending on HAVE_CERN_ROOT_SYSTEM ?

# Johannes:  put them into EXTRA_xyz_SOURCES=
# me:  That seems like it can only be part of the story
# Johannes:  ...and then pass the mysource.lo to LDADD conditionally
# me:  ah, if ROOT is available in the configure.ac?
# Johannes:  http://www.gnu.org/s/hello/manual/automake/Program-and-Library-Variables.html
#   so in configure.ac: AC_SUBST([CERN_ROOT_SYSTEM_SOURCES], [source1.lo source2.lo])
#   and in Makefile.am: xyz_LDADD+=@CERN_ROOT_SYSTEM_SOURCES@
# """
