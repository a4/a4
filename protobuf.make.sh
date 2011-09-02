#!/usr/bin/env python

from os.path import abspath, walk, join
from sys import argv

dir = abspath(argv[1])
names = []
walk(dir, lambda a,dn,fls : names.extend(join(dn,f) for f in fls if f.endswith(".proto")), None)
names = [n[len(dir)+1:-len(".proto")] for n in names]

subdirs = [sd for sd in ("/".join(n.split("/")[:-1]) for n in names) if sd]
if subdirs:
    raise Exception("No subdir support yet - subdirs will be flattened!")

lines = []
lines.append('PROTOBUF_PY=' + " ".join('$(PYDIR)/%s_pb2.py' % n for n in names))
lines.append('PROTOBUF_H=' + " ".join('$(CPPDIR)/%s.pb.h' % n for n in names))
lines.append('PROTOBUF_CC=' + " ".join('$(CPPDIR)/%s.pb.cc' % n for n in names))
lines.append('PROTOBUF_PROTO=' + " ".join('proto/%s.proto' % n for n in names))
lines.append("")
lines.append("""
# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: ${top_srcdir}/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOBUF_PROTOC} -I=${top_srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho 'class [A-Za-z0-9]*' $^ | sed 's/.py:class/ import/' | sed "s/python\/a4\/proto\/$(A4PACK)\//from ./" | sed 's/\//./g' > $@
""")
lines.append("")

print "\n".join(lines)
file("protobuf.make", "w").write("\n".join(lines))
