#!/usr/bin/env python

from os.path import abspath, walk, join
from sys import argv

dir = abspath(argv[1])
names = []
walk(dir, lambda a,dn,fls : names.extend(join(dn,f) for f in fls if f.endswith(".proto")), None)
names = [n[len(dir)+1:-len(".proto")] for n in names]
subdirs = dict((nm, "/".join(nm.split("/")[:-1])) for nm in names)

lines = []

lines.append("protodir=${localstatedir}/a4/proto/$(A4PACK)")
lines.append("protoincludedir=${includedir}/a4/proto/$(A4PACK)")
lines.append("protopythondir=${pythondir}/a4/proto/$(A4PACK)")
lines.append("")
lines.append("PYDIR=./python/a4/proto/$(A4PACK)")
lines.append("CPPDIR=./src/a4/proto/$(A4PACK)")
lines.append("PROTOBUF_CFLAGS += -I$(CPPDIR)")
lines.append("")
lines.append('PROTOBUF_PY=' + " ".join('$(PYDIR)/%s_pb2.py' % n for n in names))
lines.append('PROTOBUF_H=' + " ".join('$(CPPDIR)/%s.pb.h' % n for n in names))
lines.append('PROTOBUF_CC=' + " ".join('$(CPPDIR)/%s.pb.cc' % n for n in names))
lines.append('PROTOBUF_PROTO=' + " ".join('$(srcdir)/proto/%s.proto' % n for n in names))
lines.append("")

for subdir in set(subdirs.values()):
    if subdir:
        lines.append("proto%sdir=${protodir}/%s" % (subdir.replace("/",""), subdir))
        lines.append("protoinclude%sdir=${protoincludedir}/%s" % (subdir.replace("/",""), subdir))
        lines.append("protopython%sdir=${protopythondir}/%s" % (subdir.replace("/",""), subdir))
        lines.append("")
        lines.append("$(PYDIR)/%s/__init__.py:\n\ttouch $@" % subdir)
        lines.append("")
    lines.append(("CLEANFILES += $(PYDIR)/%s/__init__.py" % subdir).replace("//","/"))

lines.append("")

for subdir in set(subdirs.values()):
    nms_pr = " ".join('$(srcdir)/proto/%s.proto' % n for n, sd in subdirs.iteritems() if sd == subdir)
    nms_h = " ".join('$(CPPDIR)/%s.pb.h' % n for n, sd in subdirs.iteritems() if sd == subdir)
    nms_py = " ".join('$(PYDIR)/%s_pb2.py' % n for n, sd in subdirs.iteritems() if sd == subdir)
    lines.append("dist_proto%s_DATA=%s" % (subdir.replace("/",""), nms_pr))
    lines.append("nodist_protoinclude%s_HEADERS=%s" % (subdir.replace("/",""), nms_h))
    lines.append("nodist_protopython%s_PYTHON=%s $(PYDIR)/%s/__init__.py" % (subdir.replace("/",""), nms_py, subdir))
    lines[-1] = lines[-1].replace("//","/") # for subdir == ""
    lines.append("")

lines.append("""
# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: $(srcdir)/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOBUF_PROTOC} -I=$(srcdir)/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho 'class [A-Za-z0-9]*' $^ | sed 's/.py:class/ import/' | sed "s/python\/a4\/proto\/$(A4PACK)\//from ./" | sed 's/\//./g' > $@

# make sure all protobuf are generated before they are built!
BUILT_SOURCES=$(PROTOBUF_H) $(PROTOBUF_CC)

CLEANFILES += $(PROTOBUF_H) $(PROTOBUF_CC) $(PROTOBUF_PY)

""")
lines.append("")

print "\n".join(lines)
file("protobuf.make", "w").write("\n".join(lines))
