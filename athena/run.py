from math import sin, atan, atan2, exp, pi

from a4 import A4WriterStream
from aod2a4 import AOD2A4Base, athena_setup

from AthenaCommon.AppMgr import topSequence
from ROOT import gROOT, TLorentzVector

from array import array
from glob import glob

from a4.messages import Trigger, Isolation, TrackHits, MuonTrackHits
from a4.messages import Lepton, Photon, Jet, Event
from a4.messages import LorentzVector, Vertex, MissingEnergy

JETEMSCALE = 0 # http://alxr.usatlas.bnl.gov/lxr/source/atlas/Event/EventKernel/EventKernel/ISignalState.h#021

#http://alxr.usatlas.bnl.gov/lxr-stb5/source/atlas/Tracking/TrkEvent/TrkTrackSummary/TrkTrackSummary/TrackSummary.h#043
class SummaryType(object):
    numberOfContribPixelLayers      =29  #!< number of contributing layers of the pixel detector
    numberOfBLayerHits              = 0  #!< these are the hits in the first pixel layer, i.e. b-layer
    numberOfBLayerOutliers          =31  #!< number of blayer outliers  
    numberOfBLayerSharedHits        =16  #!< number of Pixel b-layer hits shared by several tracks.
    expectBLayerHit                 =42  #!< Do we expect a b-layer hit for this track?
    numberOfPixelHits               = 2  #!< these are the pixel hits, including the b-layer
    numberOfPixelOutliers           =41  #!< these are the pixel outliers, including the b-layer
    numberOfPixelHoles              = 1  #!< number of pixel layers on track with absence of hits
    numberOfPixelSharedHits         =17  #!< number of Pixel all-layer hits shared by several tracks.
    numberOfGangedPixels            =14  #!< number of pixels which have a ganged ambiguity.
    numberOfGangedFlaggedFakes      =32  #!< number of Ganged Pixels flagged as fakes
    numberOfPixelDeadSensors        =33  #!< number of dead pixel sensors crossed
    numberOfPixelSpoiltHits         =35  #!< number of pixel hits with broad errors (width/sqrt(12))
    numberOfSCTHits                 = 3  #!< number of hits in SCT
    numberOfSCTOutliers             =39  #!< number of SCT outliers
    numberOfSCTHoles                = 4  #!< number of SCT holes
    numberOfSCTDoubleHoles          =28  #!< number of Holes in both sides of a SCT module
    numberOfSCTSharedHits           =18  #!< number of SCT hits shared by several tracks.
    numberOfSCTDeadSensors          =34  #!< number of dead SCT sensors crossed
    numberOfSCTSpoiltHits           =36  #!< number of SCT hits with broad errors (width/sqrt(12))
    numberOfTRTHits                 = 5  #!< number of TRT hits
    numberOfTRTOutliers             =19  #!< number of TRT outliers
    numberOfTRTHoles                =40  #!< number of TRT holes
    numberOfTRTHighThresholdHits    = 6  #!< number of TRT hits which pass the high threshold
    numberOfTRTHighThresholdOutliers=20  #!< number of TRT high threshold outliers
    numberOfTRTDeadStraws           =37  #!< number of dead TRT straws crossed
    numberOfTRTTubeHits             =38  #!< number of TRT tube hits
# --- Muon Spectrometer
    numberOfMdtHits         = 7       # number of mdt hits
    numberOfTgcPhiHits      = 8       # tgc, rpc and csc measure both phi and eta coordinate
    numberOfTgcEtaHits      = 9 
    numberOfCscPhiHits      =10     
    numberOfCscEtaHits      =11
    numberOfRpcPhiHits      =12
    numberOfRpcEtaHits      =13
    numberOfCscEtaHoles     =21       # number of CSC Eta measurements missing from the track
    numberOfCscPhiHoles     =22       # number of CSC Phi measurements missing from the track
    numberOfRpcEtaHoles     =23       # number of RPC Eta measurements missing from the track
    numberOfRpcPhiHoles     =24       # number of RPC Phi measurements missing from the track
    numberOfMdtHoles        =25       # number of MDT measurements missing from the track
    numberOfTgcEtaHoles     =26       # number of TGC Eta measurements missing from the track
    numberOfTgcPhiHoles     =27       # number of TGC Phi measurements missing from the track
# --- all
    numberOfOutliersOnTrack =15       # number of measurements flaged as outliers in TSOS
    standardDeviationOfChi2OS = 30    # 100 times the standard deviation of the chi2 from the surfaces
# -- numbers...
    numberOfTrackSummaryTypes = 43


id_hit_names = [
    "numberOfContribPixelLayers",
    "numberOfBLayerHits",
    "numberOfBLayerOutliers",
    "numberOfBLayerSharedHits",
    "expectBLayerHit",
    "numberOfPixelHits",
    "numberOfPixelOutliers",
    "numberOfPixelHoles",
    "numberOfPixelSharedHits",
    "numberOfGangedPixels",
    "numberOfGangedFlaggedFakes",
    "numberOfPixelDeadSensors",
    "numberOfPixelSpoiltHits",
    "numberOfSCTHits",
    "numberOfSCTOutliers",
    "numberOfSCTHoles",
    "numberOfSCTDoubleHoles",
    "numberOfSCTSharedHits",
    "numberOfSCTDeadSensors",
    "numberOfSCTSpoiltHits",
    "numberOfTRTHits",
    "numberOfTRTOutliers",
    "numberOfTRTHoles",
    "numberOfTRTHighThresholdHits",
    "numberOfTRTHighThresholdOutliers",
    "numberOfTRTDeadStraws",
    "numberOfTRTTubeHits"
]

ms_hit_names = [
    "numberOfMdtHits",
    "numberOfTgcPhiHits",
    "numberOfTgcEtaHits",
    "numberOfCscPhiHits",
    "numberOfCscEtaHits",
    "numberOfRpcPhiHits",
    "numberOfRpcEtaHits",
    "numberOfCscEtaHoles",
    "numberOfCscPhiHoles",
    "numberOfRpcEtaHoles",
    "numberOfRpcPhiHoles",
    "numberOfMdtHoles",
    "numberOfTgcEtaHoles",
    "numberOfTgcPhiHoles",
]

trigger_names = {
    2010 : [
        "L1_EM14",
        "L1_EM14",
        "L1_MU10",
        "EF_e15_medium",
        "EF_mu10_MG",
        "EF_mu13_MG",
        "EF_mu13_MG_tight",
        "EF_mu18_MG",
        ],
    2011 : [
        "EF_e20_medium",
        "EF_mu20_MG",
        "EF_e10_medium_mu6",
        "EF_2mu10",
        "EF_2e12_medium"
        ]
}

def getMomentumLV(value):
    return TLorentzVector(value.px(),value.py(),value.pz(),value.e())

def make_lv(lv):
    v = LorentzVector()
    v.px, v.py, v.pz, v.e = lv.px(), lv.py(), lv.pz(), lv.e()
    return v

def make_track_hits(idtp):
    if not idtp:
        return None
    ts = idtp.trackSummary()
    if not ts:
        return None
    t = TrackHits()
    for n in id_hit_names:
        setattr(t, n, ts.get(getattr(SummaryType, n)))
    return t

def make_ms_track_hits(tp):
    if not tp:
        return None
    ts = tp.trackSummary()
    if not ts:
        return None
    t = MuonTrackHits()
    for n in ms_hit_names:
        setattr(t, n, ts.get(getattr(SummaryType, n)))
    return t

JETEMSCALE = 0 # http://alxr.usatlas.bnl.gov/lxr/source/atlas/Event/EventKernel/EventKernel/ISignalState.h#021

class AOD2A4(AOD2A4Base):

    def init(self):
        self.a4 = A4WriterStream(open(self.file_name, "w"), "Event", Event)

        import PyCintex

        PyCintex.loadDictionary("JetUtils")
        from ROOT import JetCaloHelper, JetCaloQualityUtils, Long
        self.jet_emf = lambda jet : JetCaloHelper.jetEMFraction(jet)
        self.jet_hecF = lambda jet : JetCaloQualityUtils.hecF(jet)
        self.jet_time = lambda jet : JetCaloQualityUtils.jetTime(jet)
        self.jet_quality = lambda jet : JetCaloQualityUtils.jetQualityLAr(jet)
        self.jet_fmax = lambda jet : JetCaloQualityUtils.fracSamplingMax(jet, Long())
        self.jet_jvf = lambda jet : JetCaloQualityUtils.fracSamplingMax(jet, Long())
        self.jet_bad = lambda jet : JetCaloQualityUtils.isBad(jet, False)
        self.jet_ugly = lambda jet : JetCaloQualityUtils.isUgly(jet, False)

        PyCintex.loadDictionary("egammaEnumsDict")
        PyCintex.loadDictionary("muonEventDict")
        from ROOT import MuonParameters, egammaParameters, egammaPID
        self.MuonParameters = MuonParameters
        self.egammaParameters = egammaParameters
        self.egammaPID = egammaPID

        if self.year == 2010: 
            gROOT.ProcessLine(".L checkOQ.C++")
            from ROOT import egammaOQ
            self.egOQ = egammaOQ()
            self.egOQ.initialize()

        self.tool_ttv = PyAthena.py_tool("Reco::TrackToVertex", iface="Reco::ITrackToVertex")
        self.tool_tdt = PyAthena.py_tool('Trig::TrigDecisionTool/TrigDecisionTool')
 
    def electrons(self):
        els = []
        for i, el in enumerate(self.sg["ElectronAODCollection"]):
            e = Lepton()
            e.index = i
            e.p4.CopyFrom(make_lv(el))
            assert int(el.charge()) == el.charge()
            e.charge = int(el.charge())
            #e.vertex =.recVertex().position()
            for iso in ("etcone20", "etcone30", "ptcone20", "ptcone30"):
                setattr(e.isolation, iso, el.detailValue(getattr(self.egammaParameters, iso)))

            if self.year == 2011:
                e.bad = not (el.isgoodoq(self.egammaPID.BADCLUSELECTRON) == 0)
            elif self.year == 2010:
                e.bad = not (self.egOQ.checkOQClusterElectron(167521, el.cluster.Eta(), el.cluster.Phi()) != 3)

            e.quality = e.NONE
            if el.isElectron(self.egammaPID.ElectronLoose):
                e.quality = e.LOOSE
            if el.isElectron(self.egammaPID.ElectronMedium):
                e.quality = e.MEDIUM
            if self.year == 2010:
                if el.isElectron(self.egammaPID.ElectronTight_WithTrackMatch):
                    e.quality = e.TIGHT
            else:
                if el.isElectron(self.egammaPID.ElectronTight):
                    e.quality = e.TIGHT

            trk = el.trackParticle()
            if trk:
                e.p4_track.CopyFrom(make_lv(trk))
                e.z0, e.z0err, e.d0, e.d0err = self.perigee_z0_d0(trk)
                e.track_hits.CopyFrom(make_track_hits(trk))
            if el.cluster():
                e.p4_cluster.CopyFrom(make_lv(el.cluster()))
            els.append(e)
        return els

    def muons(self):
        mus = []
        for i, mu in enumerate(self.sg["%sMuonCollection" % self.muon_algo]):
            m = Lepton()
            m.index = i
            m.p4.CopyFrom(make_lv(mu))
            assert int(mu.charge()) == mu.charge()
            m.charge = int(mu.charge())
            #m.vertex =.recVertex().position()
            for iso in ("etcone20", "etcone30", "ptcone20", "ptcone30"):
                setattr(m.isolation, iso, mu.parameter(getattr(self.MuonParameters, iso)))

            if mu.isTight() == 1:
                m.quality = m.TIGHT
            if self.muon_algo == "Muid":
                m.combined = mu.isAuthor(self.MuonParameters.MuidCo)
            elif self.muon_algo == "Staco":
                m.combined = mu.isAuthor(self.MuonParameters.STACO) and mu.isCombinedMuon()

            trk = mu.inDetTrackParticle()

            if trk:
                m.p4_track.CopyFrom(make_lv(trk))
                m.z0, m.z0err, m.d0, m.d0err = self.perigee_z0_d0(trk)
                m.track_hits.CopyFrom(make_track_hits(trk))

            ctrk = mu.combinedMuonTrackParticle()
            if ctrk:
                m.ms_hits.CopyFrom(make_ms_track_hits(ctrk))

            mus.append(m)
        return mus

    def jets(self):
        jets = []
        for i, jet in enumerate(self.sg["AntiKt4TopoEMJets"]):
            j = Jet()
            j.index = i
            j.p4.CopyFrom(make_lv(jet.hlv(JETEMSCALE)))
            #j.vertex =.recVertex().position()
            j.bad = self.jet_bad(jet)
            j.ugly = self.jet_ugly(jet)
            j.jet_vertex_fraction = self.jet_jvf(jet)
            jets.append(j)
        return jets

    def triggers(self):
        triggers = []
        for tn in trigger_names[self.year]:
            t = Trigger()
            t.name = getattr(t, tn)
            t.fired = self.tool_tdt.isPassed(tn)
            triggers.append(t)
        return triggers

    def vertices(self):
        vxs = []
        for i, vx in enumerate(self.sg["VxPrimaryCandidate"]):
            v = Vertex()
            v.index = i
            pos = vx.recVertex().position()
            v.x, v.y, v.z = pos.x(), pos.y(), pos.z()
            v.tracks = len(vx.vxTrackAtVertex())
            vxs.append(v)
        return vxs

    def perigee_z0_d0(self, trk):
        if trk and len(self.sg["VxPrimaryCandidate"]) > 0:
            vxp = self.sg["VxPrimaryCandidate"][0].recVertex().position()
            pavV0 = self.tool_ttv.perigeeAtVertex(trk, vxp)
            d0 = pavV0.parameters()[0]
            d0err = pavV0.localErrorMatrix().error(0)
            z0 = pavV0.parameters()[1]
            z0err = pavV0.localErrorMatrix().error(0)
            return z0, z0err, d0, d0err
        return None, None, None

    def met(self):
        met = MissingEnergy()
        if self.year == 2010 or self.met_loc_had_topo:
            lht = self.sg["MET_LocHadTopo"]
            reg = lht.getRegions()
            newMET_LocHadTopo_etx = reg.exReg(reg.Central) + reg.exReg(reg.EndCap) + reg.exReg(reg.Forward)
            newMET_LocHadTopo_ety = reg.eyReg(reg.Central) + reg.eyReg(reg.EndCap) + reg.eyReg(reg.Forward)
            if self.muon_algo == "Staco":
                midmet = "MET_MuonBoy"
            else:
                midmet = "MET_Muid"
            met.x = newMET_LocHadTopo_etx + self.sg[midmet].etx() - self.sg["MET_RefMuon_Track"].etx()
            met.y = newMET_LocHadTopo_ety + self.sg[midmet].ety() - self.sg["MET_RefMuon_Track"].ety()
        else:
            lht = self.sg["MET_RefFinal"]
            met.x = lht.etx()
            met.y = lht.ety()
        return met

    def execute(self):
        event = Event()
        self.load_event_info(event) # sets run_number, event_number, lumi_block and mc_event_weight
        event.triggers.extend(self.triggers())
        event.vertices.extend(self.vertices())
        event.met.CopyFrom(self.met())
        event.jets.extend(self.jets())
        event.muons.extend(self.muons())
        event.electrons.extend(self.electrons())
        self.a4.write(event)
        return PyAthena.StatusCode.Success

    def finalize(self):
        log.info("Finalizing AOD2A4")
        self.a4.close()
        return PyAthena.StatusCode.Success

if not "options" in dir():
    raise RuntimeError("No options set!")

# Set default values for testing during local running and setup athena
a_local_directory = "/data/etp"
if os.path.exists(a_local_directory):
    if "input" in options:
        input = glob(options["input"]) 
    else:
        input = glob("/data/etp/ebke/data/*109074*/*")
    athena_setup(input, 1000)
    athena_setup(input, -1)
else:
    athena_setup(None, -1)

# do autoconfiguration of input
include ("RecExCommon/RecExCommon_topOptions.py")

def get_aod2a4(year, options):
    a = AOD2A4("AOD2A4", year, options)
    a.met_loc_had_topo = True
    a.met_loc_had_topo = False
    a.muon_algo = "Staco"
    return a

ana = get_aod2a4(options["year"], options)
topSequence += ana
