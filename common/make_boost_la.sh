#!/bin/bash

set -e
set -u

prefix="$PWD/miniboost"
# generate la files for miniboost
for soname in miniboost/lib/*.so; do
    DEPS=""
    fullname=$(basename $soname .so);
    name=${fullname%%-*}
    #if test "$name" == "libboost_filesystem"; then
    #    DEPS="-lboost_system--a4-mt-1_47"
    #fi
    
    cat common/libboost-template.la | sed \
        -e "s/@FULLNAME@/$fullname/g" \
        -e "s/@NAME@/$name/g" \
        -e 's/@MAJOR@/1/g' -e 's/@MINOR@/47/g' -e 's/@REV@/0/g'\
        -e "s|@PREFIX@|$prefix/lib|" \
        -e "s/@DEPS@/$DEPS/g" > "miniboost/lib/$fullname.la"
done;

