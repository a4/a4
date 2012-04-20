A4 FAQ
======


What is A4?
-----------

A4 is a file format for encapsulating Google's protobuf messages and a
framework for efficiently processing them.

Reading and writing these files is provided by the `a4io` library, and there
are other optional libraries which provide help with analysis: `a4atlas`,
`a4hist`, `a4plot`, `a4process`, `a4root` and `a4store`. In addition, `a4store`
can optionally be used by itself with CERN ROOT.


What do I need?
---------------

GCC >= 4.4, Boost >= 1.47, Protobuf >= 2.4.1, CERN ROOT >=5 (optional)


You have all these horrible dependencies!
-----------------------------------------

> ".. isn't that a nightmare to build and maintain?"

The main real difficulty is the compiler, which if it isn't satisfied you will
likely need to ask your friendly local admins where they installed it so that
you can put it into your path. (It's available on AFS for various architectures
by tweaking this path: `/afs/cern.ch/sw/lcg/external/gcc/4.6.1/x86_64-slc5/setup.sh`)

For protobuf and boost, we provide `./get_*.sh` scripts which will automatically
compile and install these libraries. Compilation takes on the order of a few
minutes on a networked file system with a 4-core machine.

It works well. If you encounter any trouble, let us know, and people in the
future shouldn't have to suffer the same (see issues below)!

The RPATH of A4 is set correctly so LD_LIBRARY_PATH is not needed. Fully static
builds are also possible and have been tested. A fully-static `root2a4` binary
can be as small as 5MB (including statically linked root, protobuf, boost).


Do I need root permissions to install it?
-----------------------------------------

Nope. Simply configure with `./waf configure --prefix=${PWD}/install`.
The `./get_*.sh` scripts install to the a4 source directory.


What are the build times like?
------------------------------

The whole of A4 (including a4atlas) currently takes about 1 minute to compile
and install on an 8 core NFS machine.

An analysis can take anywhere from ~1 second to ~20 seconds to build. `waf` makes life
really easy, and precompiled headers can help a lot.


Why protobuf?
-------------

[protobuf](http://code.google.com/p/protobuf/) is a code generator. Given an
object structure, it can generate classes (in numerous languages) whose
instances can be easily turned into a byte string (and back) very efficiently.

"[CERN ROOT](http://root.cern.ch) has had class serialization for years!", I
hear you cry. Yes, and is an impressive feat to take arbitrary C++ classes and
serialize them to disk. There are some drawbacks to this strategy, however:

* You are tied to the whole ROOT framework
* The serialization is costly in terms of CPU time
* The structure of the data and logic code are highly intermingled
* Missing fields have to be taken care of by the user
* Adding/removing/renaming fields is difficult/messy
* Binary compatibility?

The last two can be taken care of with some form of schema evolution, but this
comes with the price of further slowdown and complexity.

In a protobuf message, fields can be omitted. This means that an empty string is
a valid protobuf message (assuming there are no "required" fields - these are
discouraged - see "[Required Is Forever](https://developers.google.com/protocol-buffers/docs/proto)").
What happens then is that `message.has_x()` returns false and `message.x()`
returns the default value, which is defined in the .proto file.

Because of this, slimming a proto file to remove fields is easy and efficient.

Because of the object structure of protobuf messages, removing objects (e.g. electrons because they failed a cut) is very easy.

A program compiled against one version of a .proto will continue to work after
addition or renaming of fields. Additionally, a program compiled against a newer
.proto is able to read old files.


What do .proto files look like?
-------------------------------

You can find examples in our [repository](https://github.com/JohannesEbke/a4/blob/master/a4atlas/proto/a4/atlas/Event.proto).
You can also define `root_prefix` and `root_branch` on fields to allow conversion to and from CERN ROOT TTrees, as in [NTUP_PHOTON](https://github.com/JohannesEbke/a4/blob/master/a4atlas/proto/a4/atlas/ntup/photon/Event.proto).


Why A4? What does it do for me?
-------------------------------

Protobuf is interesting and powerful, but it is _just_ a library to go between
bytes and class instances. It does no more than this. That means that if you want
to store more than one object, you need to write your own file format. For large
numbers of objects you will want to use compression. The bytes-on-disk for one
object are also not self describing.

A4 also serializes the class
structure itself to disk, so that a standalone .a4 file is usable by itself.
In addition, this allows one to write some generic tools, for example `a4dump`
that can be used for inspecting and aggregating information.

Metadata: In the spirit of having self-describing files, A4 has support for
metadata messages. This is a different class to the main data. The important
thing is that A4 can merge metadata messages according to some rules, or split
files based on a metadata key.

A4 has had thread safety in mind from the beginning. We also use static and
dynamic analysis tools such as clang, cppcheck and valgrind, for all of which
we are error free.

A4 files can be concatenated with the `cat` tool: This makes
merging files on the grid easier. They can then be broken apart on metadata
boundaries using `a4copy --split-per run -o output.a4 input_runs.a4`.


How difficult is it to get my existing data into the .a4 format?
----------------------------------------------------------------

A4 provides `root2a4` and `a42root` to convert betweeen flat CERN ROOT TTrees
and the .a4 format. You just need to write down the .proto file ala
[NTUP_PHOTON](https://github.com/JohannesEbke/a4/blob/master/a4atlas/proto/a4/atlas/ntup/photon/Event.proto).

(This can be done automatically for ATLAS D3PDs, see [D3PDMakerA4](https://github.com/JohannesEbke/a4/tree/master/a4atlas/D3PDMakerA4)).


How fast is it?
---------------

A4 was designed with multicore in mind. The aim is to get a linear speedup with
number of cores. These speed statistics mean that the whole event is in memory
and ready to be prodded at. Protobuf's accessors are inlined to your analysis
code, so, for example, `event.run_number()` only costs a few CPU instructions.

With ~230 filled fields/message, approximately 80 kHz / core (~50mb/sec/core)
can be achieved on an Intel Xeon E5520 @ 2.27GHz.

In my analysis I am able to process 8 GB of data (O(8M events)) in 1 minute with
a warm network disk cache - 2 minutes cold.

Please don't trust the above statistics just yet.

(TODO: put more concrete statistics here)

Note that for "generic" utilities which work on proto classes not known in
advance use a reflection API and are typically 5-10x slower.


How does the file size compare with ROOT TTrees?
------------------------------------------------

In our experience, A4 files are typically 80%-120% the size of an equivalent ROOT file.

(TODO: more concrete stats)


Compiler support?
-----------------

We use numerous features of the new C++11 standard. As such, at the moment we
depend on GCC >= 4.4. GCC 4.4 to 4.6 and clang 3 have been tested and compile
warning free.


What is the A4 library structure?
---------------------------------

A4 is split into a number of smaller libraries, with the idea that if you don't
want the full framework, `a4io` can be used independently to read/write messages.

(Library dependency graph here)


What is A4 good for?
--------------------

A4 aims to provide an easy to use system for processing large quantities of
data, and in particular deriving histograms from them.

Since the whole message must be decompressed and deserialized, you will only get
the most speed out of this format if you want to look at most variables of every
message. This is in contrast to flat ROOT tuples where one variable for many
events are next to each other on disk.

The A4 file format was originally designed with the intent of it being
streamable. However, metadata may describe events which precede it ("backwards
metadata") and in this case a seekable filesystem is required. After reading
metadata, it does no seeks, only reads.


Why reimplement histograms?
---------------------------

We wanted first to not have a hard dependency on CERN ROOT. Secondly, there was
the idea of the of the `ObjectStore` in mind. To implement that, we needed a
lightweight histogram type. The `a4results2root` provides conversion to CERN
ROOT histograms.


What is this `ObjectStore` you keep alluding to?
------------------------------------------------

The basic idea is to get rid of boilerplate and repetative code, which is found
all over the place when filling lots of histograms. Usually you have define the
histogram, create it, fill it, save it and plot it - at the very least. That
means a lot of annoying unnecessary copy-pasta strewn across many functions.

In addition, if you want to do anything extra, say, define axis labels, that is
yet more effort. Putting your histograms into a neat directory structure is also
a pain, and if there are are a dynamic number of histograms.. well. You get the
picture.

[`a4store`](https://github.com/JohannesEbke/a4/blob/master/a4store/README.md) allows you to define histogram, its labels, its position in a
directory heirarchy _and_ fill it all on the same line of code. It sounds like
it would be a mess and slow, but the penalty is
[insignificant](https://github.com/JohannesEbke/a4/blob/master/a4hist/src/gtests/test_hist.cpp)
due to compiler optimization. It looks something like this:

    Store.T<H1>("electron/pt")(100, 0, 100, "p_{T} [GeV]").fill(electron.pt());

Don't let the use of strings put you off. `a4store` can tell that they are in a
read only part of memory and only does a pointer lookup. Take a look at the
[a4store readme](https://github.com/JohannesEbke/a4/blob/master/a4store/README.md)
for more information.

I don't want to use your crazy file format but a4store sounds cool..
--------------------------------------------------------------------

> .. can I just use that?

Yes, and there are no dependancies, other than CERN ROOT! It's only O(2koc) of
code which is a much more reasonable proposition to use. Simply copy the a4store
directory and build it with `./waf`.

It's a bit of a work in progress, but you can use it with pure CERN ROOT
histograms in your analysis.


Python support?
---------------

We are diehard python fans. However, A4's python support isn't as polished as
the C++. There are a few reasons for this. With the advent of C++11, C++ is
becoming much more friendly to program in. We only use `shared<>` and `unique<>`
pointers, which behave like python's reference counted objects - this makes it
much harder to leak memory accidentally. We also have `foreach` and `foreach_enumerate`
macros  (for backwards compatibility with compilers which don't support C++11's
`for (type& x: container)`).

Another thing to note is protobuf's python support doesn't aim for speed, only
simplicity of implementation. A4 was built with speed in mind, so the python
version doesn't get as much attention because it is hundreds of times slower.


Where is documentation for X?
-----------------------------

X =

* [protobuf](https://developers.google.com/protocol-buffers/docs/proto)
* [protobuf generated code documentation](https://developers.google.com/protocol-buffers/docs/reference/cpp-generated)
* [boost](http://www.boost.org/doc/libs/1_49_0/libs/libraries.htm)
* [a4 user](http://www.ebke.org/a4/)
* [a4 dev](http://www.ebke.org/a4-dev/)
* [waf](http://docs.waf.googlecode.com/git/book_16/single.html)


I can't find my variable! It's right _there_ in the protobuf!
-------------------------------------------------------------

Variables in the generated C++ protobuf classes are lowercase. Please read the
[generated code documentation](https://developers.google.com/protocol-buffers/docs/reference/cpp-generated).


Why can't I extend the C++ classes which protobuf generates?
------------------------------------------------------------

The philosophy of protobuf is to separate the concerns of data and the
interpretation of it. This means you know what you're getting when you ask for a
member of a protobuf class, which can be invaluable when it comes to writing
clean code.


I have an issue!
----------------

If you have a problem of any kind, however small it seems, please let us know by
[submitting an issue](https://github.com/JohannesEbke/a4/issues/new).


I'd like to contribute / I have an idea
---------------------------------------

Please, create an [issue](https://github.com/JohannesEbke/a4/issues/new), then [fork](http://help.github.com/fork-a-repo/) us on github and commit! (You don't need our
permission). If you have an idea we would be glad to give pointers. If it is not
a small idea come and talk to us first.

What things have you forgotten to mention in detail in this FAQ?
----------------------------------------------------------------

* `a4process`
* protobuf's extensions
* Example analyses
* Analysis workflow
