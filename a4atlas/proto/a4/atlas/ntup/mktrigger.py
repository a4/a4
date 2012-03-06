#! /usr/bin/env python

from sys import argv
from textwrap import dedent

from ROOT import TFile

def main():
    tupname, treename, filename = argv[1:4]

    f = TFile(filename)
    tree = f.Get(treename)
    
    leaves = [l.GetName() for l in tree.GetListOfLeaves()]
    
    def mktrigger(prefix):
        def fmt(name):
            pfx, _, name = name.partition("_")
            assert pfx == prefix
            name, rootname = name, name
            if name[0].isdigit():
                name = "_" + name
            return ('optional bool {0} = {{0}} '
                     '[default=false, (root_branch)="{1}"];').format(name, rootname)
    
        return [fmt(name) for name in leaves if name.startswith(prefix)]
        
    def mkproto(name, triggers):
        triggers = "\n    ".join(t.format(i) for i, t in enumerate(triggers, 1))
        content = dedent("""
            package a4.root.atlas.{tupname};
            import "a4/root/RootExtension.proto";
            
            message {name} {{
                {triggers}
            }}
        """).strip().format(name=name, triggers=triggers, tupname=tupname)
        with open("{0}/{1}.proto".format(tupname, name), "w") as fd:
            fd.write(content)
            
    mkproto("L1", mktrigger("L1"))
    mkproto("L2", mktrigger("L2"))
    mkproto("EF", mktrigger("EF"))
    
if __name__ == "__main__":
    main()
