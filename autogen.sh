#!/bin/sh

if test -n `ls gtest`; then git submodule update --init gtest; fi

if autoreconf -i --force; then
    rm -rf autom4te.cache/
    echo "------------------------------------------------------------"
    echo "A4: AutoMake Setup completed."
    echo "A4: Disregard messages about LT_INIT and AC_CONFIG_MACRO_DIR" 
    echo "A4: These macros are in included files and are therefore not picked up." 
    echo "------------------------------------------------------------"

else
    echo "A4: autogen.sh failed."
fi
