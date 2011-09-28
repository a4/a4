from sys import argv

from yaml import load_all

class D3PDVariable(object):
    def __init__(self, args):
        self.typecode, self.name, self.primitive, self.has_root_tclass = args
        

class D3PDObject(object):
    def __init__(self, args):
        self.prefix = args["prefix"] or ""
        self.classname = args["classname"]
        if self.classname.endswith("D3PDObject"):
            self.classname = self.classname[:-len("D3PDObject")]
        self.is_container = args["is_container"]
        print args["variables"]
        self.variables = map(D3PDVariable, args["variables"])
    
    def write(self):
        print "message {0} {{".format(self.classname)
        for v in self.variables:
            print '    optional {0} = # [(branch_name) = "{1}{0}"];'.format(v.name, self.prefix)

class ProtoFile(object):
    def add_object(self, o):
        pass

def generate_proto(input_stream):
    p = ProtoFile()
    for obj in load_all(input_stream):
        p.add_object(D3PDObject(obj))

def main():
    for filename in argv[1:]:
        with open(filename) as fd:
            generate_proto(fd)
