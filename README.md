A4 - An Analysis Tool for High-Energy Physics
---------------------------------------------

A4 is a data processing format, tool-set and library which provides fast I/O
of structured data using the Google protocol buffer library:
http://code.google.com/p/protobuf/

This is currently an experimental project, but has already been successfully
applied to physics analysis.

For instructions on how to install A4, see INSTALL. 
More documentation is in the [doc/](http://liba4.net/a4/blob/master/doc/) directory. 
See also the [FAQ](http://liba4.net/a4/blob/master/doc/FAQ.md).
An overview of the structure and a preliminary look at the performance is available at
[arxiv:1208.1600](http://arxiv.org/abs/1208.1600).

If you are impatient, try:

    git clone https://github.com/a4/a4.git
    cd a4
    ./waf go
    source install/bin/this_a4.sh
    cd tutorial/

For a quick start install A4 and take a look at the examples
in the tutorial directory.

Inspired and building on Samvel Khalatyans [RTvsPBThreads](https://github.com/ksamdev/RTvsPBThreads)


Overview
--------

## Modular organization

 * a4io provides fast I/O of protobuf events to a custom file format
 * a4process contains processor classes with overridable event loops
 * a4hist contains fast and easy to use histogramming classes
 * a4store provides a way to define and fill histograms on one line in the loop
 * a4root contains conversion routines and tools from a4 to ROOT and back
 * a4plot is a container module for ROOT plotting tools
 * a4atlas contains ATLAS-specific event and metadata definitions and tools

## Key features

 * Store protobuf messages of arbitrary types
 * Stores the description of messages, making the format self-describing
 * Transparent compression using different algorithms (zlib, gzip and snappy)
 * Store metadata messages for blocks of events
 * Binary concatenation of A4 files yields a valid A4 file with all metadata
 * Support linear no-seeking mode of operation, suitable for network streaming
 * Tries to minimize manual book-keeping
