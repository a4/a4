from D3PDMakerA4.A4DumpAlg import A4DumpAlg
from D3PDMakerA4.D3PDMakerA4Conf import D3PD__A4D3PDSvc
service = D3PD__A4D3PDSvc(
    "A4ReaderD3PDSvc",
    Mode="A4ProtoDumpD3PD",
    OutputLevel=INFO,
    #Mode="A4DumpD3PD", # Generate A4 itself (not yet implemented)
)
ServiceMgr += service

from OutputStreamAthenaPool.MultipleStreamManager import MSMgr
OrigRootStream = MSMgr.NewRootStream

a4algs = []

def a4stream(name, file, typename):
    root_stream = OrigRootStream(name, file, typename)
    print "-"*80
    print "OVERRIDE: ", name, file, typename
    print "-"*80
    d3pdalg = A4DumpAlg("A4DumpAlg"+name, Directory="./", TuplePath="./", D3PDSvc=service, orig_stream=root_stream)
    a4algs.append(d3pdalg)
    return d3pdalg

MSMgr.NewRootStream = a4stream
