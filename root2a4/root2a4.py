#! /usr/bin/env python

import ROOT as R

from minty.treedefs.egamma import egamma_wrap_tree

from a4 import A4WriterStream

from a4.messages import ShowerShape, Trigger, Isolation, TrackHits, MuonTrackHits
from a4.messages import Electron, Muon, Photon, Jet, Event
from a4.messages import LorentzVector, Vertex, MissingEnergy

def object_maker(new, old):
    """
    Makes a `new` for every object in `old`
    """
    for o in old:
        n = new.add()
        yield n, o

tmplv = R.TLorentzVector()
def map_fourvector(new, old):
    tmplv.SetPtEtaPhiE(old.pt, old.eta, old.phi, old.E)
    v = new
    v.px, v.py, v.pz, v.e = tmplv.Px(), tmplv.Py(), tmplv.Pz(), tmplv.E()
    return v

def map_showershapes(new, old):
    fields = list(getattr(ShowerShape, ShowerShape._DESCRIPTOR_KEY).fields_by_name)
    
    shower_shape = new.shower_shape
    for f in fields:
        if hasattr(old, f):
            setattr(shower_shape, f, getattr(old, f))
    
def map_egamma(new, old):
    map_fourvector(new.p4, old)
    map_fourvector(new.p4_cluster, old.cl)
    map_showershapes(new, old)
    #new.cluster_time = old.time

def map_quality(Type, new, old):    
    
    for q in ["tight", "medium", "loose"]:
        tightness = getattr(Type, q.upper(), None)
        if tightness:
            setattr(new, q, getattr(old, q))
            
    if hasattr(Type, "dq"):
        for q in ["OQ", "goodOQ", "OQRecalc"]:
            setattr(new.dq, q, getattr(old, q))

def map_photons(objects):
    for new, old in objects:
        map_egamma(new, old)
        map_quality(Photon, new, old)

def map_hits(new, old):
    if hasattr(old, "nPixHits"): new.numberOfPixelHits = old.nPixHits
    if hasattr(old, "nSCTHits"): new.numberOfSCTHits = old.nSCTHits

def map_electrons(objects):
    for new, old in objects:
        map_egamma(new, old)
        map_quality(Electron, new, old)
        map_hits(new.track_hits, old)
        
def map_triggers(Nevent, event):
    Nevent.triggers.add(name=Trigger.EF_2g20_loose, fired=event.EF._2g20_loose)

def map_vertices(n, o):
    for v in o.vertices:
        n.vertices.add(tracks=v.nTracks, z=v.z)

def map_event(event):
    n = Nevent = Event()
    o = event
        
    n.run_number, n.lumi_block, n.event_number = o.RunNumber, o.LumiBlock, o.EventNumber
    
    map_vertices(n, o)
    map_triggers(n, o)
    
    map_photons  (object_maker(Nevent.photons,   event.photons))
    map_electrons(object_maker(Nevent.electrons, event.electrons))
    
    return Nevent
    
def main():
    class options:
        release = "rel16"
        project = "data11"
        events = None
        output = "output.root"
        grl_path = None
        skip = 0
        limit = 100
        run_specific_output = False
        have_metadata = False
        dump = False

    f = ("/scratch/pwaller/data/data11/50/skim.slim/"
         "user.PeterWaller.002737._00001.outputXYZ.rootXYZ.tgz.root")
    file_name = "test.a4"
    
    ph = R.TChain("photon")
    ph.Add(f)
    print "Will copy {0} events:", ph.GetEntries()    
        
    tree = egamma_wrap_tree(ph, options)
    
    with A4WriterStream(open(file_name, "w"), "Event", Event) as a4w:
        def copy(index, event):
            if not index % 100:
                print index
            a4w.write(map_event(event))
        i = tree.loop(copy, hi=5000)
            
    print "Copied {0} events".format(i)
    
if __name__ == "__main__":
    main()
