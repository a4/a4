#! /usr/bin/env python

import re

from difflib import SequenceMatcher
from sys import argv
from textwrap import dedent

from yaml import load_all

# Groups of things which should be considered similar for grouping purposes
similarity_groups = [
    ["x", "y", "z"],
    ["pt", "eta", "phi", "e", "m"],
    ["loose", "medium", "tight", "mediumiso", "tightiso"],
    ["ethad", "ethad1", "e033", "f1", "f1core", "emins1", "fside", "emax2", 
     "ws3", "wstot", "e132", "e1152", "emaxs1", "deltaes", "e233", "e237", "e277",
     "weta2", "f3", "f3core", "eratio", "etoverpt", "reta", "rphi", "dphi", "deta"],
    ["zvertex", "errz"],
]

MANUAL_FIXUP = {
    "RunNumber": "run_number",
    "EventNumber": "event_number",
    "jet_AntiKt4TopoEMJets_": "jets_4",
    "jet_AntiKt6TopoEMJets_": "jets_6",
    "trig_L2_trk_idscan_eGamma_": "trig_in_det_tracks_idscan",
    "trig_L2_trk_sitrack_eGamma_": "trig_in_det_tracks_sitrack",
    "trig_EF_emcl_slw_": "clusters_slw",    
}

def space_out_camel_case(stringAsCamelCase):
    """
    Note to self: There has to be a better way.
    """
    part = []
    parts = [part]
    for this, next in zip(stringAsCamelCase, stringAsCamelCase[1:] + "#"):
        part.append(this)
        if this.islower() != next.islower():
            part = []
            parts.append(part)
    parts = ["".join(p) for p in parts if p]
    new_parts = []
    for this, next in zip(parts[::2], parts[1::2] + [""]):
        new_parts.append(this + next)
    
    return "_".join(s.lower() for s in new_parts)

def build_similarity_tester():
    """
    Note that elements can't belong to multiple similarity groups!
    """
    lookup = {}
    for group in similarity_groups:
        lower = [s.lower() for s in group]
        s = set(lower)
        for element in group:
            lookup[element] = s
        
    def test_similarity(left, right):
        return right.lower() in lookup.get(left.lower(), set())
    return test_similarity

TEST_SIMILAR = build_similarity_tester()
ALL_TYPES_SEEN = set()

def similarity(prev, this):
    if TEST_SIMILAR(prev, this): return 1
    return SequenceMatcher(None, prev, this).ratio()

class D3PDVariable(object):
    def __init__(self, args, parent):
        self.typecode, self.name, self.primitive, self.has_root_tclass = args
        self.parent = parent
    
    @property
    def prefix(self):
        return self.parent.prefix
        

class D3PDObject(object):
    def __init__(self, args):
        self.prefix = args["prefix"] or ""
        self.classname = args["classname"]
        if self.classname.endswith("D3PDObject"):
            self.classname = self.classname[:-len("D3PDObject")]
        self.is_container = args["is_container"]
        self._variables = args["variables"] or []
        self.variables = [D3PDVariable(v, self) for v in self._variables]
        
        # Ignore the "n" counter variables
        self.variables = [v for v in self.variables if v.name != "n"]

class VariableBase(object):
    label, type, name, number, extra, comment = "optional", "unk", "unk", "#", "", ""

TYPECODE_MAP = {
    "I": "int32",
    "i": "uint32",
    "F": "float",
    "D": "double",
    "c": "string",
    "O": "bool",
}

TYPE_MAP = {
    "int": "int32",
    "float": "float",
    "double": "double",
    "short": "int32",
    "unsigned int": "uint32",
    "unsigned short": "uint32",
    "std::string": "string",
}

def is_vector(t):
    if "vector<" in t:
        t, _, _ = t[t.index("vector<") + len("vector<"):].partition(",")
        return True, t
    return False, t

def type_name(t):
    if len(t) == 1:
        return 'optional', TYPECODE_MAP.get(t, "unk-{0}".format(t))
    
    vector, subtype = is_vector(t)
    ALL_TYPES_SEEN.add(subtype)
    
    if vector:
        subvector, subsubtype = is_vector(subtype)
        if subvector:
            return 'repeated', TYPE_MAP[subsubtype]
        return 'optional', TYPE_MAP[subtype]
        
    assert False, "I don't know how to convert things which aren't vectors."
    
class VariablePlain(VariableBase):
    """
    Represents a plain-old-data variable
    """
    def __init__(self, var, parent, defaults):
        self.name = MANUAL_FIXUP.get(var.name, var.name)
        self.label, self.type = type_name(var.typecode)
        self.parent = parent
        default = ""
        name = parent.classname + "." + self.name
        if name in defaults:
            best_count, best_value = defaults[name][-1]
            if isinstance(best_value, bool):
                best_value = str(best_value).lower()
            if best_count > 10:
                default = ', default={0}'.format(best_value)
        self.extra = ' [(root_branch)="{1}"{2}]'.format(var.prefix, var.name, default)
        
class VariableMessage(VariableBase):
    """
    Represents an (optionally) repeated message variable
    """
    def __init__(self, f):
        obj = f.obj
        self.name = space_out_camel_case(obj.classname)
        
        if self.name == obj.classname:
            self.name = obj.classname.lower()
        
        assert self.name != obj.classname, "{0} {1}".format(self.name, obj.classname)
        if obj.is_container:
            if obj.prefix in MANUAL_FIXUP:
                self.name = MANUAL_FIXUP[obj.prefix]
                
            elif self.name.endswith("y"): # Pluralize
                self.name = self.name[:-1] + "ies"
            elif self.name.endswith("s"):
                self.name += "es"
            else:
                self.name += "s"
            self.label = "repeated"
            self.extra = ' [(root_prefix)="{0}"]'.format(obj.prefix)
        self.type = obj.classname

class ProtoFile(object):
    def __init__(self, obj):
        self.obj = obj
        self.package = None
        self.name = obj.classname
        self.children = []
        self.extensions = []
    
    @property
    def filename(self):
        return "{0}.proto".format(self.name)
    
    @property
    def content_variables(self):
        PROTO_VARIABLE = dedent("""
            {v.label} {v.type} {v.name} = {count}{v.extra};{v.comment}
        """).strip()
        
        variables = []
        extend, append = variables.extend, variables.append
        newline = lambda: append("")
        
        count = 1
        from json import load
        defaults = load(open("defaults"))
        
        # Us and our extensions
        for e in [self.obj] + self.extensions:
            newline()
            prev = None
            for v in e.variables:
                if prev and similarity(prev.name, v.name) < 0.4:
                    newline()
                    count += 100
                    count = count - (count % 100)
                s = similarity(prev.name if prev else "", v.name)
                append(PROTO_VARIABLE.format(v=VariablePlain(v, self.obj, defaults), s=s, count=count))
                prev = v
                count += 1
        
        newline()
        
        # Child objects
        for c in self.children:
            append(PROTO_VARIABLE.format(v=VariableMessage(c), count=count))
            count += 1
        
        newline()
            
        return "\n    ".join(variables)
        
    @property
    def content(self):
        PROTO_HEADER = dedent("""
            import "a4/root/RootExtension.proto";
        """).strip()
    
        PROTO_INCLUDES = dedent("""
            import "a4/root/atlas/ntup_photon/{0}";
        """).strip()

        PROTO_MESSAGE = dedent("""
            message {m.name} {{
                {m.content_variables}
            }}
        """).strip()
        
        includes = sorted(set(c.filename for c in self.children))
        return "\n".join(
            ([] if not self.package else ["package {0};".format(self.package)]) +
            [PROTO_HEADER] +
            [PROTO_INCLUDES.format(name) for name in includes] +
            [""] +
            [PROTO_MESSAGE.format(m=self)]
        )
    
    def append(self, rhs):
        self.children.append(rhs)
        
    def extend(self, rhs):
        self.extensions.append(rhs)

def generate_proto(input_stream):
    class Event:
        classname = "Event"
        variables = []
        
    event = ProtoFile(Event)
    files, file_map = [event], {'': event}
    
    # TODO: Also need a mapping between classname (+ crosscheck)
    
    for i, obj in enumerate(load_all(input_stream)):
        obj = D3PDObject(obj)
        print "Read one: {0:20s} {1}".format(obj.classname, obj.prefix)
        
        if not obj.variables:
            print " -- skipping"
            continue
        
        if obj.prefix in file_map:
            
            file_map[obj.prefix].extend(obj)
            continue
        
        f = file_map[obj.prefix] = ProtoFile(obj)
        event.append(f)
        files.append(f)
        
        #if i > 10: break
        
    return files
    

def main():
    for filename in argv[1:]:
        with open(filename) as fd:
            files = generate_proto(fd)
            
    for output in files:
        if output.name == "Event":
            output.package = "a4.root.atlas.ntup_photon"
            
        with open("ntup_photon/" + output.filename, "w") as fd:
            fd.write(output.content)
    
    print "All types seen:"
    print " ", "\n  ".join(sorted(ALL_TYPES_SEEN))

if __name__ == "__main__":
    main()
