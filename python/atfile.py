
from os.path import join as pjoin
from ROOT import TFile
class ATFile:
    def __init__(self, fn):
        self.fn = fn
        self.tfile = TFile(fn)
        self.dir = {}
        self.scan(self.tfile)

    def scan(self, dir, path=""):
        for key in list(dir.GetListOfKeys()):
            name = key.GetName()
            cname = key.GetClassName()
            if cname == "TDirectoryFile":
                self.dir[pjoin(path,name)] = self.scan(dir.GetDirectory(name), pjoin(path, name))
            else:
                self.dir[pjoin(path,name)] = cname

    def find_type(self, ttype, dir=None, path=""):
        return [k for k, v in self.dir.iteritems() if v == ttype]

    def get(self, h):
        return self.tfile.Get(h)
