A4 - An Analysis Tool for High-Energy Physics
=============================================
Johannes Ebke <ebke@cern.ch>


Prerequisites
-------------

There are two main libraries which are required by A4:
 * Google Protocol Buffers (protobuf, http://code.google.com/p/protobuf/)
 * C++ Boost Library (http://www.boost.org/) version >= 1.41

Since you might not have these installed on your system, A4 provides scripts
that download them for you. They will then be compiled and installed
automatically together with A4 as 'builtin' libraries:

./get_protobuf.sh
./get_miniboost.sh


Source
------

Currently the best way to get an up-to-date A4 is either to clone the
git repository with:
git clone https://github.com/JohannesEbke/a4.git

or obtain a tarball of the current master at:
https://github.com/JohannesEbke/a4/tarball/master

Installing
----------

To install A4 into 'my/software/a4' in your home directory, type:

./waf configure build install --prefix=$HOME/my/software/a4

To add this installation to the PYTHONPATH, PATH and compiler flags, go to the
installation directory and run:

source bin/this_a4.sh

Now you can go into the tutorial/ directory in the A4 source distribution and
all the examples should work with the provided Makefiles - try it!

NOTE: In the tutorial the ntup-swmz Event is used. To enable this you need to 
install a4 with the cofigure option --enable-atlas-ntup=smwz. type:
./waf configure --prefix=$HOME/my/software/a4 --enable-atlas-ntup=smwz 
./waf build install
Additional information is available in the tutorial source files.

The generated executables (except for the python ones) should also work if you
have not sourced this_a4.sh.

