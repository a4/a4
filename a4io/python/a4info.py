#!/usr/bin/env python
from sys import argv, exit
from a4.stream import InputStream

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-g", "--grl", default=False, help="create a GRL from the given files", metavar="grl.xml")
parser.add_option("-m", "--metadata", action="store_true", help="print metadata from file")
parser.add_option("-f", "--footer", action="store_true", help="print metadata in footer")
(options, args) = parser.parse_args()

if len(args) == 0:
    parser.print_help()
    exit(-1)

run_infos = {}

def cname(id, stream):
    cls = stream.pool.find_class_id(id)
    if cls is None:
        return "<unknow class %i>" % id
    else:
        return stream.pool.class_ids[id].__name__


for fn in args:
    print("%s: " % fn)
    r = InputStream(open(fn))
    print("%s" % r.info())
    if options.metadata:
        for md in r.metadata.values():
            print md

    if options.footer:
        counts = {}
        for f in r.footers.itervalues():
            for cc in f.class_count:
                nm = cname(cc.class_id, r)
                counts[nm] = cc.count + counts.get(nm, 0)
        if len(counts) == 0:
            print "No event count information in file found! (Probably python generated)"
        else:
            print "Object counts:"
            print "--------------"
            maxlen = max(map(len, counts.keys()))
            for n in sorted(counts.keys()):
                print "%s: %i" % (n + " "*(maxlen-len(n)), counts[n])
                    
    if options.grl:
        for md in r.metadata.values():
            if md.simulation:
                raise Exception("File '%s' contains simulation data - cannot create GRL from this!" % fn)
            for ri in md.lumiblock:
                run_infos.setdefault(ri.run, set()).add(ri.lumiblock)

if options.grl:

    grl_start = "<LumiRangeCollection><NamedLumiRange><Name>A4</Name><Version>2.1</Version>"
    runlist = '<Metadata Name="RunList">%s</Metadata>' % ",".join(map(str, sorted(run_infos)))

    lbcs = []
    for run in sorted(run_infos): 
        lbcs.append("<LumiBlockCollection><Run>%i</Run>" % run)

        startlb = None
        lastlb = None
        for lb in sorted(run_infos[run]):
            if startlb is None:
                startlb = lb
                lastlb = lb
            elif lb == lastlb + 1:
                lastlb = lb
            else:
                lbcs.append('    <LBRange Start="%i" End="%i"/>' % (startlb, lastlb))
                startlb = lb
                lastlb = lb

        if not startlb is None:
            lbcs.append('    <LBRange Start="%i" End="%i"/>' % (startlb, lastlb))
                

        lbcs.append("</LumiBlockCollection>")

    grl_end = "</NamedLumiRange></LumiRangeCollection>"

    file(options.grl, "w").write("\n".join((grl_start, runlist, "\n".join(lbcs), grl_end)))

