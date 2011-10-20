#! /usr/bin/env python

import re

from difflib import SequenceMatcher
from os.path import exists
from sys import argv
from textwrap import dedent

from yaml import load_all

# Groups of things which should be considered similar for grouping purposes
# Most are autodetected using difflib.SequenceMatcher, but some need help.
SIMILARITY_GROUPS = [
    ["x", "y", "z"],
    ["lbn", "bcid"],
    ["pt", "eta", "phi", "theta", "e", "m", "px", "py", "pz"],
    ["loose", "medium", "tight", "looseiso", "mediumiso", "tightiso", "loosepp", 
     "mediumpp", "tightpp", "loosear", "looseariso", "tightar", "tightariso"],
    ['deltaes', 'deta', 'dphi', 'e033', 'e1152', 'e132', 'e233', 'e237', 'e277', 
     'ehad1', 'emax2', 'emaxs1', 'emins1', 'eratio', 'ethad', 'ethad1', 
     'etoverpt', 'f1', 'f1core', 'f3', 'f3core', 'fracs1', 'fside', 'hadet1', 
     'hadet', 'reta', 'rphi', 'weta2', 'ws3', 'wstot'],
    ["zvertex", "errz"],
    ["chi2", "ndf", "ndof"],
    ["type", "origin", "typebkg", "originbkg"],
    ["cellmaxfrac", "longitudinal", "secondlambda", "lateral", "secondr", 
     "centerlambda"],
    ["issimulation", "iscalibration", "istestbeam"],
]

DISSIMILARITY_GROUPS = [
    ["phis3", "isem"],
]

# If this much of the beginning/end of the string match, consider them part
# of the same group.
# PREFIX_SIMILAR_LEN, SUFFIX_SIMILAR_LEN = 4, 4

MANUAL_FIXUP = {
    "RunNumber": "run_number",
    "EventNumber": "event_number",
    "jet_AntiKt4TopoEMJets_": "jets_4",
    "jet_AntiKt6TopoEMJets_": "jets_6",
    "trig_L2_trk_idscan_eGamma_": "trig_in_det_tracks_idscan",
    "trig_L2_trk_sitrack_eGamma_": "trig_in_det_tracks_sitrack",
    "trig_EF_emcl_": "clusters_emcl",
    "trig_EF_emcl_slw_": "clusters_emcl_slw",
    "jet_AntiKt4TruthJets_": "truth_jets_4",
    "jet_AntiKt6TruthJets_": "truth_jets_6",
    "jet_AntiKt4TruthWithMuNoIntJets_": "truth_jets_4_mu_noint",
    "jet_AntiKt6TruthWithMuNoIntJets_": "truth_jets_6_mu_noint",
}

# Mappings from prefixes to class names
MANUAL_CLASSNAME_FIXUP = {
    "PhotonPV_": "PhotonPrimaryVertex",
    "jet_AntiKt4TruthJets_" : "JetTruth",
    "met_": "MET",
    "el_as_conv_ph_": "ElAsPh",
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

def build_similarity_tester(similarity_groups):
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
        left_parts = left.split("_")
        right_parts = right.split("_")
        prefix_or_suffix_matches = False
        if len(left_parts) > 1 and len(right_parts) > 1:
            prefix_or_suffix_matches = (
                left_parts[0] == right_parts[0]
                or left_parts[-1] == right_parts[-1])
            
        return (right.lower() in lookup.get(left.lower(), set()) 
                 or prefix_or_suffix_matches)
    return test_similarity

TEST_SIMILAR = build_similarity_tester(SIMILARITY_GROUPS)
TEST_DISSIMILAR = build_similarity_tester(DISSIMILARITY_GROUPS)
ALL_TYPES_SEEN = set()

def test_similar_but_not_dissimilar(prev, this):
    #if TEST_DISSIMILAR(prev, this):
        #return False
    return TEST_SIMILAR(prev, this)
    # and not 

def similarity(prev, this, this_group):
    if test_similar_but_not_dissimilar(prev, this): return 1
    for element in this_group:
        if test_similar_but_not_dissimilar(element, this):
            return True
    return SequenceMatcher(None, prev, this).ratio()

class D3PDVariable(object):
    def __init__(self, args, parent):
        self.typecode, self.name, self.primitive, self.has_root_tclass = args
        self.name = self.name.replace("::", "_")
        self.parent = parent        

class D3PDObjectClass(object):
    def __init__(self, args):
        self.name = args["classname"].replace("::", "_")
        if self.name.endswith("D3PDObject"):
            self.name = self.name[:-len("D3PDObject")]
        self.is_container = args["is_container"]
        self._variables = args["variables"] or []
        self.variables = [D3PDVariable(v, self) for v in self._variables]
        
        # Ignore the "n" counter variables
        self.variables = [v for v in self.variables if v.name != "n"]

class VariableBase(object):
    label, type, name, number, extra, comment = "optional", "unk", "unk", "#", "", ""

TYPECODE_MAP = {
    "B": "int32",
    "b": "uint32",
    "S": "int32",
    "s": "uint32",
    "I": "int32",
    "i": "uint32",
    "F": "float",
    "D": "double",
    "O": "bool",
    "C": "string",
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
        self.orig_name = var.name
        self.name = MANUAL_FIXUP.get(var.name, var.name)
        self.label, self.type = type_name(var.typecode)
        if not parent.is_container and is_vector(var.typecode)[0]:
            self.label = "repeated"
        self.parent = parent
        default = ""
        name = parent.name + "." + self.name
        if name in defaults:
            best_count, best_value = defaults[name][-1]
            if isinstance(best_value, bool):
                best_value = str(best_value).lower()
            if best_count > 10:
                default = ', default={0}'.format(best_value)
        self.extra = ' [(root_branch)="{0}"{1}]'.format(var.name, default)
        
class VariableMessage(VariableBase):
    """
    Represents an (optionally) repeated message variable
    """
    def __init__(self, f, prefix, cls):
        self.name = space_out_camel_case(cls.name)
        self.orig_name = prefix
        
        if self.name == cls.name:
            self.name = cls.name.lower()
        
        # Ensure that the name is different to the class name
        assert self.name != cls.name, "{0} {1}".format(self.name, cls.name)
        self.name = MANUAL_FIXUP.get(prefix, self.name)
        
        if cls.is_container:
            if prefix in MANUAL_FIXUP:
                "Don't correction"
            elif self.name.endswith("y"): # Pluralize
                self.name = self.name[:-1] + "ies"
            elif self.name.endswith("s"):
                self.name += "es"
            elif self.name.endswith("ex"):
                self.name =  self.name[:-len("ex")] + "ices"
            else:
                self.name += "s"
            self.label = "repeated"
            
        self.extra = ' [(root_prefix)="{0}"]'.format(prefix)
        self.type = cls.name

class ProtoFile(object):
    def __init__(self, cls):
        self.cls = cls
        self.children = []
        self.extensions = []
    
    @property
    def name(self):
        return self.cls.name
    
    @property
    def filename(self):
        return "{0}.proto".format(self.name)
    
    @property
    def content_variables(self):
        PROTO_VARIABLE = dedent("""
            {v.label} {v.type} {v.name} = {count}{v.extra};{v.comment}
        """).strip()
        
        content = []
        extend, append = content.extend, content.append
        newline = lambda: append("")
        
        count = 1
        if exists("defaults"):
            from json import load
            defaults = load(open("defaults"))
        else:
            defaults = {}
        
        variables = []
        
        # Us and our extensions
        for extension in [self.cls] + self.extensions:
            for variable in extension.variables:
                variables.append(VariablePlain(variable, self.cls, defaults))
        
        # Subclasses 
        for prefix, cls in self.children:
            variables.append(VariableMessage(self, prefix, cls))
        
        collisions = {}
        for variable in variables:
            collisions.setdefault(variable.name, []).append(variable)
        
        # Fix collisions
        for name, vs in sorted(collisions.iteritems()):
            if len(vs) <= 1:
                # It's okay, we only have one name
                continue

            # collision because there are different prefixes...
            if len(set(v.orig_name for v in vs)) > 1:
                for v in vs:
                    v.name = v.orig_name.lower().rstrip("_")

            for orig_name in set(v.orig_name for v in vs):
                cols = [v for v in vs if v.orig_name == orig_name]
                if len(cols) > 1:
                    # real collision!
                    def tup(s):
                        return s.label, s.type, s.extra
                    if len(set(map(tup, cols))) != 1:
                        print "Incompatible variables in Class of the same name:"
                        for c in cols:
                            print "%s %s %s %s" % (s.label, s.type, orig_name, s.extra)
                        raise RuntimeError("Incompatible variables in Class of the same name:")
                    else:
                        for c in cols[1:]:
                            variables.remove(c)

        this_group = set()
        
        # Generate the text (body of the message) for these variables
        prev = None
        for variable in variables:            
            if not variable:
                continue
                       
            if prev and similarity(prev.name, variable.name, this_group) < 0.4:
                this_group = set()
                newline()
                count += 100
                count = count - (count % 100)
            
            this_group.add(variable.name)
            append(PROTO_VARIABLE.format(v=variable, count=count))
            prev = variable
            count += 1
                    
        return "\n    ".join(content)
        
    def content(self, package_name):
        PROTO_HEADER = dedent("""
            import "a4/root/RootExtension.proto";
        """).strip()
    
        PROTO_INCLUDES = dedent("""
            import "a4/root/atlas/{0}/{1}.proto";
        """).strip()

        PROTO_MESSAGE = dedent("""
            message {m.name} {{
                {m.content_variables}
            }}
        """).strip()
        
        includes = sorted(set(c.file.name for _, c in self.children))
        return "\n".join(
            ["package a4.root.atlas.{0};".format(package_name)] +
            [PROTO_HEADER] +
            [PROTO_INCLUDES.format(package_name, name) for name in includes] +
            [""] +
            [PROTO_MESSAGE.format(m=self)]
        )
    
    def append(self, rhs):
        self.children.append(rhs)
        
    def extend(self, rhs):
        self.extensions.append(rhs)

def generate_proto(input_stream):
    class Event:
        name = "Event"
        variables = []
        is_container = False
        
    event = ProtoFile(Event)
    files, file_map, file_cls_map = [event], {'': event}, {}
    
    # TODO: Also need a mapping between classname (+ crosscheck)
        
    from pprint import pprint
        
    class_lookup = {}
        
    for i, obj in enumerate(load_all(input_stream)):
        #pprint(obj)
        if not obj["variables"]:
            print " -- skipping", obj["classname"], "because no variables"
            continue
            
        prefix = obj["prefix"] or ""
        
        varset = frozenset(map(tuple, obj["variables"]))
        if varset in class_lookup:
            cls = class_lookup[varset]
        else:
            cls = class_lookup[varset] = D3PDObjectClass(obj)
        print "Read one: {0:20s} {1}".format(cls.name, prefix)
        
        if cls.name.startswith("D3PDObject"):
            if prefix in MANUAL_CLASSNAME_FIXUP:
                cls.name = MANUAL_CLASSNAME_FIXUP[prefix]
        
        if prefix in file_map:
            # This shares a prefix with an existing object, so it extends the
            # fields of that object
            file_map[prefix].extend(cls)
            cls.file = file_map[prefix]
            continue

        if cls.name in file_cls_map:
            # This shares a name with an existing object, so it extends the
            # fields of that object
            file_cls_map[cls.name].extend(cls)
            cls.file = file_cls_map[cls.name]
            event.append((prefix, cls))
            continue
        
        cls.file = f = file_map[prefix] = ProtoFile(cls)
        file_cls_map[cls.name] = f
        event.append((prefix, cls))
        files.append(f)
        
        #if i > 10: break
        
    return files
    

def main():
    #package_name = "ntup_photon"
    
    package_name = argv[1]

    for filename in argv[2:]:
        with open(filename) as fd:
            files = generate_proto(fd)
            
    for output in files:
        with open(package_name + "/" + output.filename, "w") as fd:
            fd.write(output.content(package_name))
    
    print "All types seen:"
    print " ", "\n  ".join(sorted(ALL_TYPES_SEEN))

if __name__ == "__main__":
    main()
