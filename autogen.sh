#!/bin/sh
if autoreconf -i --force; then
    rm -rf autom4te.cache/
    echo "A4: Disregard messages about LT_INIT and AC_CONFIG_MACRO_DIR" 
    echo "A4: These macros are in included files and are therefore not picked up." 
    echo "A4: AutoMake Setup completed."
else
    echo "A4: autogen.sh failed."
fi
