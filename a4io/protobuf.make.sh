#!/usr/bin/env python

from os.path import abspath, walk, join
from sys import argv

dir = abspath(argv[1])
names = []
walk(dir, lambda a,dn,fls : names.extend(join(dn,f) for f in fls if f.endswith(".proto")), None)
names = [n[len(dir)+1:-len(".proto")] for n in names]

lines = []
lines.append('PROTOBUF_PY=' + " ".join('$(PYDIR)/%s_pb2.py' % n for n in names))
lines.append('PROTOBUF_H=' + " ".join('$(CPPDIR)/%s.pb.h' % n for n in names))
lines.append('PROTOBUF_CC=' + " ".join('$(CPPDIR)/%s.pb.cc' % n for n in names))
lines.append('PROTOBUF_PROTO=' + " ".join('proto/%s.proto' % n for n in names))
lines.append("")

print "\n".join(lines)
file("protobuf.make", "w").write("\n".join(lines))
