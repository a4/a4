#!/usr/bin/env python

from os.path import abspath, walk, join
from sys import argv

dir = "proto"
names = []
walk(dir, lambda a,dn,fls : names.extend(join(dn,f) for f in fls if f.endswith(".proto")), None)
names = [n[len(dir)+1:-len(".proto")] for n in names]
subdirs = dict((nm, "/".join(nm.split("/")[:-1])) for nm in names)
if subdirs:
    l, top_subdir = max((len(s),s) for s in subdirs.values())

lines = []
lines.append("@A4_SET_AM_V_PROTOC@")
#lines.append("protodir=${localstatedir}/a4/$(A4PACK)")
#lines.append("protoincludedir=${includedir}/a4/$(A4PACK)")
#lines.append("protopythondir=${pythondir}/a4/$(A4PACK)")
lines.append("protodir=${localstatedir}")
lines.append("protoincludedir=${includedir}")
lines.append("protopythondir=${pythondir}")
lines.append("")
lines.append("PROTOSRCDIR=./proto")
lines.append("PYDIR=./python")
lines.append("CPPDIR=./src")
lines.append("PROTOBUF_CFLAGS += -I$(CPPDIR)/a4/$(A4PACK)")
lines.append("")
lines.append('PROTOBUF_PY=' + " ".join('$(PYDIR)/%s_pb2.py' % n for n in names))
lines.append('PROTOBUF_H=' + " ".join('$(CPPDIR)/%s.pb.h' % n for n in names))
lines.append('PROTOBUF_CC=' + " ".join('$(CPPDIR)/%s.pb.cc' % n for n in names))
lines.append('PROTOBUF_PROTO=' + " ".join('$(PROTOSRCDIR)/%s.proto' % n for n in names))
lines.append("")

all_subdirs = set()

for sd in subdirs.values():
    all_subdirs.add(sd)
    for i in range(1,sd.count("/")):
        all_subdirs.add("/".join(sd.split("/")[:-i]))
        subdirs[sd] = []

for subdir in all_subdirs:
    lines.append("proto%sdir=${protodir}/%s" % (subdir.replace("/",""), subdir))
    lines.append("protoinclude%sdir=${protoincludedir}/%s" % (subdir.replace("/",""), subdir))
    lines.append("protopython%sdir=${protopythondir}/%s" % (subdir.replace("/",""), subdir))
    lines.append("")
    lines.append(("CLEANFILES += $(PYDIR)/%s/__init__.py" % subdir).replace("//","/"))
    lines.append("")
    #if subdir.count("/") > 1:
    lines.append("$(PYDIR)/%s/__init__.py:\n\ttouch $@" % subdir)

lines.append("")

for subdir in all_subdirs:
    nms_pr = " ".join('$(PROTOSRCDIR)/%s.proto' % n for n, sd in subdirs.iteritems() if sd == subdir)
    nms_h = " ".join('$(CPPDIR)/%s.pb.h' % n for n, sd in subdirs.iteritems() if sd == subdir)
    nms_py = " ".join('$(PYDIR)/%s_pb2.py' % n for n, sd in subdirs.iteritems() if sd == subdir)
    lines.append("dist_proto%s_DATA=%s" % (subdir.replace("/",""), nms_pr))
    lines.append("nodist_protoinclude%s_HEADERS=%s" % (subdir.replace("/",""), nms_h))
    lines.append("nodist_protopython%s_PYTHON=%s $(PYDIR)/%s/__init__.py" % (subdir.replace("/",""), nms_py, subdir))
    lines[-1] = lines[-1].replace("//","/") # for subdir == ""
    lines.append("")

lines.append("""
# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: $(PROTOSRCDIR)/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	$(AM_V_PROTOC)${PROTOBUF_PROTOC} -I$(srcdir)/proto -I${PROTOBUF_INCLUDE} --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
#$(PYDIR)/a4/$(A4PACK)/__init__.py: $(PROTOBUF_PY)
#	$(AM_V_PROTOC)grep -Ho 'class [A-Za-z0-9]*' $^ | sed 's/.py:class/ import/' | sed 's/python\//from /' | sed 's/\//./g' > $@

# make sure all protobuf are generated before they are built!
BUILT_SOURCES=$(PROTOBUF_H) $(PROTOBUF_CC)

CLEANFILES += $(PROTOBUF_H) $(PROTOBUF_CC) $(PROTOBUF_PY)

""")
lines.append("")

#print "\n".join(lines)
file("protobuf.am", "w").write("\n".join(lines))
