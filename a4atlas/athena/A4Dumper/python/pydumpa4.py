from math import sin, atan, atan2, exp, pi
import gc

from a4.stream import OutputStream
from aod2a4 import AOD2A4Base, athena_setup

from AthenaCommon.AppMgr import topSequence
from ROOT import gROOT, TLorentzVector

from array import array
from glob import glob

from a4.atlas.Trigger_pb2 import Trigger, TriggerFeature
from a4.atlas.Isolation_pb2 import Isolation
from a4.atlas.TrackHits_pb2 import TrackHits, MuonTrackHits
from a4.atlas.Electron_pb2 import Electron
from a4.atlas.Muon_pb2 import Muon
from a4.atlas.Photon_pb2 import Photon
from a4.atlas.Jet_pb2 import Jet
from a4.atlas.Event_pb2 import Event, Track, TruthParticle
from a4.atlas.EventStreamInfo_pb2 import EventStreamInfo
from a4.atlas.Physics_pb2 import LorentzVector, Vertex, MissingEnergy, Perigee

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
        "L1_EM14",

        "L1_MU10",
        "L1_MU11",
        "L1_MU15",
        "L1_MU20",

        # Level 2 muon triggers
        "L2_mu15",
        "L2_mu15_medium",

        "L2_mu18",
        "L2_mu18_medium",
        "L2_mu18_MG",
        "L2_mu18_MG_medium",
        
        "L2_mu20",
        "L2_mu20_medium",
        "L2_mu20_MG",
        "L2_mu20_MG_medium",

        "L2_mu22",
        "L2_mu22_medium",
        "L2_mu22_MG",
        "L2_mu22_MG_medium",

        "L2_mu40_MSonly_barrel",
        "L2_mu40_MSonly_barrel_medium",

        # Single-electron Trigger
        "EF_e15_medium",
        "EF_e20_medium",
        "EF_e22_medium",
        "EF_e22_medium1",
        "EF_e22vh_medium1",
        "EF_e40_medium1",
        "EF_e45_medium1",
        
        # Di- and Multielectron Triggers
        "EF_2e10_medium",
        "EF_2e12_medium",
        "EF_2e12T_medium",
        "EF_2e12Tvh_medium",
        "EF_2e15_medium",
        "EF_2e15vh_medium",
        
        # Gamma triggers
        "EF_g20_loose",
        "EF_g40_loose",
        "EF_g60_loose",
        "EF_g80_loose",
        "EF_g100_loose",
        "EF_2g20_loose",
        
        # Single Muon Trigger
        "EF_mu10_MG",
        "EF_mu13_MG",
        "EF_mu13_MG_tight",
        "EF_mu15i",
        "EF_mu18",
        "EF_mu18_medium",
        "EF_mu18_MG",
        "EF_mu18_MG_medium",
        "EF_mu20_MG",
        "EF_mu20i_medium",
        "EF_mu20_empty",
        "EF_mu40_MSonly",
        "EF_mu40_MSonly_tight",
        "EF_mu40_MSonly_barrel",
        "EF_mu40_MSonly_barrel_medium",
        
        # Dimuon triggers
        "EF_2mu10",
        "EF_2mu10_loose",
        "EF_mu15_mu10_EFFS",
        "EF_mu15_mu10_EFFS_medium",
        
        # e/mu triggers
        "EF_e10_medium_mu6"
    ],

    2012 : [
        # level 1
        "L1_MU15",
        
        # level 2
        "L2_mu24_tight",
        "L2_mu36_tight",

        # Single-electron Trigger
        "EF_e22vh_medium1",
        "EF_e22vhi_medium1",
        "EF_e24vh_medium1",
        "EF_e24vhi_medium1",
        "EF_e45_medium1",
        "EF_e60_medium1",

        # Multi-electorn tirgger
        "EF_e24vh_medium1_e7_medium1",
        "EF_2e12Tvh_loose1",

        # Gamma triggers
        "EF_g20_loose",
        "EF_g40_loose",
        "EF_g60_loose",
        "EF_g80_loose",
        "EF_g100_loose",
        "EF_g120_loose",
        "EF_g200_etcut",
        
        # single muon
        "EF_mu24i_tight",
        "EF_mu24_tight",
        "EF_mu36i_tight",
        "EF_mu50_MSonly_barrel_tight", # this propably L2 ???
        # lowest unprescaled triggers single muon
        "EF_mu20i_tight",
        "EF_mu18_medium",
        "EF_mu24_medium",
        "EF_mu36_tight",
        "EF_mu40_MSonly_barrel_tight",

        # multi muon
        "EF_2mu13",
        "EF_mu18_tight_mu8_EFFS",
        "EF_mu24_tight_mu6_EFFS",
        "EF_3mu6",
        "EF_3mu6_MSonly",
        "EF_mu18_tight_2mu4_EFFS",
        # combiend [muon + jet + met]
        "EF_2mu4T_xe60_tclcw",
        "EF_2mu8_EFxe40_tclcw",
        "EF_mu4T_j65_a4tchad_xe60_tclcw_loose",
        "EF_mu24_j65_a4tchad_EFxe40_tclcw",
        "EF_mu24_tight_4j35_a4tchad",
        # combined [muon + e/gamma + tau]
        "EF_e12Tvh_medium1_mu8",
        "EF_mu18_tight_e7_medium1",
        "EF_2e7T_loose1_mu6",
        "EF_mu24_g20_medium1",
        "EF_7T_loose1_2mu6",
        "EF_mu18_2g10_medium1",
        "EF_3mu6tau20_medium1_mu15", 
        
        # W tag and probe triggers 
        "EF_e13_etcutTrk_xs60",
        "EF_e20_etcutTrk_xe20",
        "EF_e13_etcutTrk_xs60_dphi2j15xe20",
        "EF_e20_etcutTrk_xs60_dphi2j15xe20"
        ]
}

topoisolations = ("topoetcone20", "topoetcone30", "topoetcone40")
isolations = ("etcone20", "etcone30", "etcone40", "ptcone20", "ptcone30", "ptcone40")

useSpectrometerTrack = 0 #/< Use the track reconstructed in the MS
useExtrapolatedTrack = 1 #/< Use the track extrapolated to the IP
useCombinedTrack = 2 #/< Use the combined track
useMuGirlTrack = 3 #/< Use a "MuGirl" track (for _MG chains)


def getMomentumLV(value):
    return TLorentzVector(value.px(),value.py(),value.pz(),value.e())

def make_lv(lv):
    v = LorentzVector()
    v.px, v.py, v.pz, v.e = lv.px(), lv.py(), lv.pz(), lv.e()
    return v

def set_lv(v, lv):
    v.px, v.py, v.pz, v.e = lv.px(), lv.py(), lv.pz(), lv.e()

def make_vertex(vx):
    v = Vertex()
    v.x, v.y, v.z = vx.x(), vx.y(), vx.z()
    return v

def set_track_hits(t, idtp):
    if not idtp:
        return False
    ts = idtp.trackSummary()
    if not ts:
        return False
    for n in id_hit_names:
        idx = getattr(SummaryType, n)
        val = ts.get(idx)
        if int(val) != -1:
            setattr(t, n, val)
    return True

def make_ms_track_hits(tp):
    if not tp:
        return None
    ts = tp.trackSummary()
    if not ts:
        return None
    t = MuonTrackHits()
    for n in ms_hit_names:
        idx = getattr(SummaryType, n)
        val = ts.get(idx)
        if int(val) != -1:
            setattr(t, n, val)
    return t

def set_truth(p, p4, charge, pdg_id=None):
    p.p4.CopyFrom(make_lv(p4))
    p.charge = charge
    if pdg_id:
        p.pdg_id = pdg_id
    return p

def set_met_contrib(mc, weight):
    if weight:
        mc.wet = weight.wet()
        mc.wpx = weight.wpx()
        mc.wpy = weight.wpy()
        mc.status_word = weight.statusWord()
        #print mc.status_word, mc.wet, mc.wpx, mc.wpy

JETEMSCALE = 0 # http://alxr.usatlas.bnl.gov/lxr/source/atlas/Event/EventKernel/EventKernel/ISignalState.h#021

class AOD2A4(AOD2A4Base):
    try_hfor = True

    def init(self):
        self.a4 = OutputStream(open(self.file_name, "w"), "AOD2A4", Event, EventStreamInfo)

        import PyCintex
        PyCintex.loadDictionary("TrigMuonEvent")
        PyCintex.loadDictionary("TrigObjectMatching")
        self.tmefih = PyCintex.makeClass("TrigMatch::TrigMuonEFInfoHelper")

        from ROOT import vector

        PyCintex.loadDictionary("JetUtils")

        from ROOT import JetCaloHelper, JetCaloQualityUtils, Long, CaloSampling
        self.jet_emf = lambda jet : JetCaloHelper.jetEMFraction(jet)
        self.jet_hecF = lambda jet : JetCaloQualityUtils.hecF(jet)
        ### smax needed for jet cealning
        #### FIX THIS: don't know either getNumberOfSamplings() or Unknown
        #### UPDATE: getNumberOfSamplings just returns Unknown!
        self.jet_smax = Long(CaloSampling.getNumberOfSamplings())
        self.jet_fmax = lambda jet : JetCaloQualityUtils.fracSamplingMax(jet, Long(CaloSampling.Unknown))
        self.jet_time = lambda jet : JetCaloQualityUtils.jetTimeCells(jet)
        self.jet_quality_lar = lambda jet : JetCaloQualityUtils.jetQualityLAr(jet)
        self.jet_quality_hec = lambda jet : JetCaloQualityUtils.jetQualityHEC(jet)

        self.jet_bad = lambda jet : JetCaloQualityUtils.isBad(jet, False)
        self.jet_ugly = lambda jet : JetCaloQualityUtils.isUgly(jet, False)

        PyCintex.loadDictionary("egammaEnumsDict")
        PyCintex.loadDictionary("muonEventDict")
        PyCintex.loadDictionary("egammaAnalysisUtils")
        
        PyCintex.loadDictionary("MissingETEvent")
        from ROOT import MuonParameters, egammaParameters, egammaPID
        from ROOT import ElectronMCChargeCorrector
        self.MuonParameters = MuonParameters
        self.egammaParameters = egammaParameters
        self.egammaPID = egammaPID
        self.empp_helper = PyCintex.makeClass("isEMPlusPlusHelper")

        if self.year == 2010: 
            gROOT.ProcessLine(".L checkOQ.C++")
            from ROOT import egammaOQ
            self.egOQ = egammaOQ()
            self.egOQ.initialize()

        self.tool_ttv = PyAthena.py_tool("Reco::TrackToVertex", iface="Reco::ITrackToVertex")
        self.tool_tdt = PyAthena.py_tool('Trig::TrigDecisionTool/TrigDecisionTool')
        self.tool_tmt = PyAthena.py_tool("TrigMatchTool/TrigMatchTool")
        self.tool_hfor= PyAthena.py_tool("HforTool",iface="IHforTool")
        #self.tool_extrap = PyAthena.py_tool("Trk::Extrapolator/AtlasExtrapolator",iface="Trk::IExtrapolator")
        #import ROOT
        #self.PerigeeSurface = ROOT.__getattr__("Trk::PerigeeSurface")

        #self.tight_same_count = 0
        #self.only_othertight_count = 0
        #self.only_tight_count = 0
        #self.event_counter = 0
 
    def tracks(self, pb):
        for i, trk in enumerate(self.sg["TrackParticleCandidate"]):
            if abs(trk.pt()) < 15000:
                continue
            if abs(trk.pt()) < 500 or abs(trk.eta()) > 2.5:
                continue
            ts = trk.trackSummary()
            if not ts:
                continue
            if ts.get(SummaryType.numberOfPixelHits) < 1:
                continue
            if ts.get(SummaryType.numberOfSCTHits) < 4:
                continue
            t = pb.add()
            set_lv(t.p4, trk)
            t.charge = int(trk.charge())

            vx = trk.reconstructedVertex()
            if vx:
                t.vertex_index = self.PV.index(vx)
            set_track_hits(t.hits, trk)
            self.perigee_z0_d0(t.perigee, trk)

    def electrons(self):
        els = []
        for i, el in enumerate(self.sg["ElectronAODCollection"]):
            e = Electron()
            e.index = i
            e.p4.CopyFrom(make_lv(el))
            assert int(el.charge()) == el.charge()
            e.charge = int(el.charge())

            if el.cluster():
                e.eta_s2 = el.cluster().etaBE(2)
            shower = el.detail("EMShower")
            if shower:
                e.eta_pointing = shower.parameter(self.egammaParameters.etap)

            vx = el.origin()
            if vx:
                #if vx.position():
                #    e.vertex.CopyFrom(make_vertex(vx.position()))
                e.vertex_index = self.PV_rec.index(vx)
            e.author = el.author()
            for iso in isolations:
                setattr(e.isolation, iso, el.detailValue(getattr(self.egammaParameters, iso)))
            for iso in topoisolations:
                setattr(e.isolation, iso, el.detailValue(getattr(self.egammaParameters, iso)))

            if self.year == 2011:
                e.bad_oq = not (el.isgoodoq(self.egammaPID.BADCLUSELECTRON) == 0)
            elif self.year == 2010:
                e.bad_oq = not (self.egOQ.checkOQClusterElectron(167521, el.cluster.Eta(), el.cluster.Phi()) != 3)


            if self.year == 2010:
                e.loose = bool(el.isElectron(self.egammaPID.ElectronLoose))
                e.medium = bool(el.isElectron(self.egammaPID.ElectronMedium))
                e.tight = bool(el.isElectron(self.egammaPID.ElectronTight_WithTrackMatch))

            if self.year == 2011:
                e.loose = bool(el.passID(self.egammaPID.ElectronIDLoose))
                e.medium = bool(el.passID(self.egammaPID.ElectronIDMedium))
                e.tight = bool(el.passID(self.egammaPID.ElectronIDTight))
                e.loose_pp = bool(el.passID(self.egammaPID.ElectronIDLoosePP))
                e.medium_pp = bool(el.passID(self.egammaPID.ElectronIDMediumPP))
                e.tight_pp = bool(el.passID(self.egammaPID.ElectronIDTightPP))

            if self.year == 2012:
                e.bad_oq = not (el.isgoodoq(self.egammaPID.BADCLUSELECTRON) == 0)
                if (el.trackParticle()):
                    e.loose_pp = self.empp_helper.IsLoosePlusPlus(el)
                    e.medium_pp = self.empp_helper.IsMediumPlusPlus(el)
                    e.tight_pp = self.empp_helper.IsTightPlusPlus(el)
                
            trk = el.trackParticle()
            if trk:
                e.p4_track.CopyFrom(make_lv(trk))
                self.perigee_z0_d0(e.perigee, trk)
                self.perigee_z0_d0_unbiased(e.perigee_unbiased, trk)
                set_track_hits(e.track_hits, trk)
            if el.cluster():
                e.p4_cluster.CopyFrom(make_lv(el.cluster()))

            #if trk:
            #    chns = self.tool_tmt.__getattribute__("chainsPassedByObject<TrigElectron, INavigable4Momentum>")(trk, 0.15)
            #    for chain in list(chns):
            #        if chain in trigger_names[self.year]:
            #            e.matched_trigger.append(getattr(Trigger,chain))
            #    chns.clear()

            if self.metref_composition.contains(el):
                set_met_contrib(e.met_contribution, self.metref_composition.getParameter(el))

            els.append(e)
        return els

    def muons(self, muon_algo):
        mus = []
        for i, mu in enumerate(self.sg["%sMuonCollection" % muon_algo]):
            m = Muon()
            m.index = i
            m.p4.CopyFrom(make_lv(mu))
            assert int(mu.charge()) == mu.charge()
            m.charge = int(mu.charge())
            m.author = mu.author()
            vx = mu.origin()
            if vx:
                #if vx.position():
                #    m.vertex.CopyFrom(make_vertex(vx.position()))
                m.vertex_index = self.PV_rec.index(vx)
            for iso in isolations:
                setattr(m.isolation, iso, mu.parameter(getattr(self.MuonParameters, iso)))

            m.tight = (mu.isTight() == 1)
            m.segment_tagged = mu.isSegmentTaggedMuon()
            if muon_algo == "Muid":
                m.combined = mu.isAuthor(self.MuonParameters.MuidCo)
            elif muon_algo == "Staco":
                m.combined = mu.isAuthor(self.MuonParameters.STACO) and mu.isCombinedMuon()

            trk = mu.inDetTrackParticle()
            if trk:
                m.p4_track.CopyFrom(make_lv(trk))
                self.perigee_z0_d0(m.perigee_id, trk)
                self.perigee_z0_d0_unbiased(m.perigee_unbiased, trk)
                set_track_hits(m.track_hits, trk)
    
            ms_trk = mu.muonExtrapolatedTrackParticle()
            if ms_trk:
                m.p4_ms.CopyFrom(make_lv(ms_trk))

            ctrk = mu.combinedMuonTrackParticle()
            if ctrk:
                self.perigee_z0_d0(m.perigee_cmb, trk)
                m.ms_hits.CopyFrom(make_ms_track_hits(ctrk))

            #m.matched_trigger_efi_ms.extend(self.matched_chains(mu, useSpectrometerTrack))
            #m.matched_trigger_efi_ex.extend(self.matched_chains(mu, useExtrapolatedTrack))
            #m.matched_trigger_efi_cb.extend(self.matched_chains(mu, useCombinedTrack))
            #m.matched_trigger_efi_mg.extend(self.matched_chains(mu, useMuGirlTrack))
            #A17 m.matched_trigger_efi_mgt.extend(self.matched_chains(mu, useMuGirlTagTrack))
            #chns = self.tool_tmt.__getattribute__("chainsPassedByObject<CombinedMuonFeature, INavigable4Momentum>")(mu,0.1)
            #for chain in list(chns):
            #    if chain in trigger_names[self.year]:
            #        m.matched_trigger_cmf.append(getattr(Trigger,chain))
            #chns.clear()
            #chns = self.tool_tmt.__getattribute__("chainsPassedByObject<TrigMuonEF, INavigable4Momentum>")(mu,0.1)
            #for chain in list(chns):
            #    if chain in trigger_names[self.year]:
            #        m.matched_trigger_ef.append(getattr(Trigger,chain))
            #chns.clear()
            #chns = self.tool_tmt.__getattribute__("chainsPassedByObject<MuonFeature, INavigable4Momentum>")(mu,0.1)
            #for chain in list(chns):
            #    if chain in trigger_names[self.year]:
            #        m.matched_trigger_mf.append(getattr(Trigger,chain))
            #chns.clear()
            if self.metref_composition.contains(mu):
                set_met_contrib(m.met_contribution, self.metref_composition.getParameter(mu))
            mus.append(m)
        return mus

    def jets(self, container):
        jets = []
        for i, jet in enumerate(self.sg[container]):
            j = Jet()
            j.index = i
            j.p4.CopyFrom(make_lv(jet))
            j.vertex_index = int(jet.getMoment("OriginIndex"))
            #vx = self.sg["VxPrimaryCandidate"][j.vertex_index]
            #j.vertex.CopyFrom(make_vertex(vx.recVertex().position()))
            j.bad = self.jet_bad(jet)
            j.ugly = self.jet_ugly(jet)
            j.jet_vertex_fraction = jet.getMoment("JVF")
            j.n_trk = jet.getMoment("nTrk")

            j.p4_em.CopyFrom(make_lv(jet.hlv(JETEMSCALE)))
            j.EMJES = jet.getMoment("EMJES")
            j.SV0 = jet.getFlavourTagWeight("SV0")
            j.SV1 = jet.getFlavourTagWeight("SV1")
            j.IP3D = jet.getFlavourTagWeight("IP3D")
            j.JetFitterCOMBNN = jet.getFlavourTagWeight("JetFitterCOMBNN")
            #j.MV1 = jet.getFlavourTagWeight("MV1") # not yet in data
            cc = jet.getMoment("BCH_CORR_CELL")
            if cc != 0:
                j.BCH_CORR_CELL = cc
            cj = jet.getMoment("BCH_CORR_JET")
            if cj != 0:
                j.BCH_CORR_JET = cj
            #http://alxr.usatlas.bnl.gov/lxr/source/atlas/PhysicsAnalysis/D3PDMaker/JetD3PDMaker/src/JetTrueTagFillerTool.cxx#045
            tti = jet.tagInfo("TruthInfo")
            if tti:
                tl = tti.jetTruthLabel()
                j.truth_flavor = j.Light
                if tl == "C":
                    j.truth_flavor = j.C
                if tl == "B":
                    j.truth_flavor = j.B
                if tl == "T":
                    j.truth_flavor = j.T

            j.lar_quality = jet.getMoment("LArQuality")
            j.hec_quality = jet.getMoment("HECQuality")
            j.negative_e = jet.getMoment("NegativeE")
            j.emf = self.jet_emf(jet)
            j.hecf = self.jet_hecF(jet)
            j.timing = jet.getMoment("Timing")
            j.fmax = self.jet_fmax(jet)
            j.smax = self.jet_smax
            j.sum_pt_trk = jet.getMoment("sumPtTrk")
            j.avg_lar_qf = jet.getMoment("AverageLArQF")
            j.eta_origin = jet.getMoment('EtaOrigin')
            j.phi_origin = jet.getMoment('PhiOrigin')
            j.m_origin = jet.getMoment('MOrigin')


            if self.metref_composition.contains(jet):
                set_met_contrib(j.met_contribution, self.metref_composition.getParameter(jet))

            jets.append(j)
        return jets

    def matched_chains(self, mu, which_track):
        self.tmefih.setTrackToUse(which_track)
        chains = []
        cpbo = self.tool_tmt.__getattribute__("chainsPassedByObject<TrigMuonEFInfo, INavigable4Momentum>")(mu, 0.1)
        for chain in list(cpbo):
            if chain in trigger_names[self.year]:
                chains.append(getattr(Trigger, chain))
        cpbo.clear()
        return chains
        
    def triggers(self, pb):
        for tn in trigger_names[self.year]:
            if self.tool_tdt.isPassed(tn):
                #try: getattr(t,tn)
                #except: continue                    
                t = pb.add()
                t.name = getattr(t, tn)
                t.fired = True
                c_eg  = self.tool_tmt.__getattribute__("getTriggerObjects<egamma>")(tn, True)
                c_te  = self.tool_tmt.__getattribute__("getTriggerObjects<TrigElectron>")(tn, True)
                c_tp  = self.tool_tmt.__getattribute__("getTriggerObjects<TrigPhoton>")(tn, True)
                c_tme = self.tool_tmt.__getattribute__("getTriggerObjects<TrigMuonEF>")(tn, True)
                c_mroi= self.tool_tmt.__getattribute__("getTriggerObjects<Muon_ROI>")(tn, True)
                c_mf  = self.tool_tmt.__getattribute__("getTriggerObjects<MuonFeature>")(tn, True)
                c_cmf = self.tool_tmt.__getattribute__("getTriggerObjects<CombinedMuonFeature>")(tn, True)
                c_tmei= self.tool_tmt.__getattribute__("getTriggerObjects<TrigMuonEFInfo>")(tn, True)

                eg, te, tp, tme, mroi, mf, cmf, tmei = map(list, (c_eg, c_te, c_tp, c_tme, c_mroi, c_mf, c_cmf, c_tmei))

                tmeit = sum((list(efi.TrackContainer()) for efi in tmei), [])
                tmeit_ms = [tr.SpectrometerTrack() for tr in tmeit if tr.MuonType() == 1 and tr.hasSpectrometerTrack()]
                tmeit_ex = [tr.ExtrapolatedTrack() for tr in tmeit if tr.MuonType() == 1 and tr.hasExtrapolatedTrack()]
                tmeit_cb = [tr.CombinedTrack() for tr in tmeit if tr.MuonType() == 1 and tr.hasCombinedTrack()]
                tmeit_mg = [tr.CombinedTrack() for tr in tmeit if tr.MuonType() == 2 and tr.hasCombinedTrack()]

                def make_tf(pb, feature):
                    if feature:
                        ff = pb.add()
                        ff.eta = feature.eta()
                        ff.phi = feature.phi()
                        ff.pt = feature.pt()

                [make_tf(t.features_egamma, f) for f in eg]
                [make_tf(t.features_trig_electron, f) for f in te]
                [make_tf(t.features_trig_photon, f) for f in tp]
                [make_tf(t.features_trig_muon_ef, f) for f in tme]
                [make_tf(t.features_trig_muon_efi_ms, f) for f in tmeit_ms]
                [make_tf(t.features_trig_muon_efi_ex, f) for f in tmeit_ex]
                [make_tf(t.features_trig_muon_efi_cb, f) for f in tmeit_cb]
                [make_tf(t.features_trig_muon_efi_mg, f) for f in tmeit_mg]
                [make_tf(t.features_muon_roi, f) for f in mroi]
                [make_tf(t.features_muon, f) for f in mf]
                [make_tf(t.features_muon_combined, f) for f in cmf]

                for vec in (c_te, c_tp, c_tme, c_mf, c_cmf, c_tmei):
                    vec.clear()

    def vertices(self):
        vxs = []
        for i, vx in enumerate(self.PV):
            v = Vertex()
            v.index = i
            pos = vx.recVertex().position()
            v.x, v.y, v.z = pos.x(), pos.y(), pos.z()
            v.tracks = len(vx.vxTrackAtVertex())
            vxs.append(v)
        return vxs

    def perigee_z0_d0(self, p, trk):
        if trk and len(self.PV) > 0:
            res = self.tool_ttv.d0z0AtVertex(trk, self.PV_rec[0].position())
            p.d0, p.d0err = res.first.first, res.first.second
            p.z0, p.z0err = res.second.first, res.second.second

    def perigee_z0_d0_unbiased(self, p, trk):
        if trk and len(self.PV) > 0:
            #print "!!!!!!!"
            #print type(self.PV)
            #print type(self.PV[0])
            #print type(self.PV_rec)
            #print type(self.PV_rec[0])
            res = self.tool_ttv.d0z0AtVertex_unbiased(trk, self.PV[0])
            p.d0, p.d0err = res.first.first, res.first.second
            p.z0, p.z0err = res.second.first, res.second.second
            
#    def extrap_perigee_z0_d0(self, p, trk):
#        mp = trk.measuredPerigee()
#        vxp = self.PV_rec[0].position()
#        ps = self.PerigeeSurface(vxp)
#        if mp and vxp and ps:
#            pavV0 = self.tool_extrap.extrapolateDirectly(mp, ps)
#            p.d0 = pavV0.parameters()[0]
#            p.d0err = pavV0.localErrorMatrix().error(0)
#            p.z0 = pavV0.parameters()[1]
#            p.z0err = pavV0.localErrorMatrix().error(1)
#
#    def ttv_perigee_z0_d0(self, p, trk):
#        if trk and len(self.PV) > 0:
#            vxp = self.PV_rec[0].position()
#            pavV0 = self.tool_ttv.perigeeAtVertex(trk, vxp)
#            p.d0 = pavV0.parameters()[0]
#            p.d0err = pavV0.localErrorMatrix().error(0)
#            p.z0 = pavV0.parameters()[1]
#            p.z0err = pavV0.localErrorMatrix().error(0)
#            del pavV0
            

    def met_reffinal45(self):
        met = MissingEnergy()
        lht = self.sg["MET_RefFinal"]
        reg = lht.getRegions()
        met.x = reg.exReg(reg.Central) + reg.exReg(reg.EndCap) + reg.exReg(reg.Forward)
        met.y = reg.eyReg(reg.Central) + reg.eyReg(reg.EndCap) + reg.eyReg(reg.Forward)
        met.sum = reg.etSumReg(reg.Central) + reg.etSumReg(reg.EndCap) + reg.etSumReg(reg.Forward)
        met.met_central.x = reg.exReg(reg.Central); 
        met.met_central.y = reg.eyReg(reg.Central); 
        met.met_central.sum = reg.etSumReg(reg.Central); 
        met.met_endcap.x = reg.exReg(reg.EndCap); 
        met.met_endcap.y = reg.eyReg(reg.EndCap); 
        met.met_endcap.sum = reg.etSumReg(reg.EndCap); 
        met.met_forward.x = reg.exReg(reg.Forward); 
        met.met_forward.y = reg.eyReg(reg.Forward);
        met.met_forward.sum = reg.etSumReg(reg.Forward);
        return met

    def met_lochadtopo(self, muon_algo = "Staco"):
        met = MissingEnergy()
        lht = self.sg["MET_LocHadTopo"]
        reg = lht.getRegions()
        newMET_LocHadTopo_etx = reg.exReg(reg.Central) + reg.exReg(reg.EndCap) + reg.exReg(reg.Forward)
        newMET_LocHadTopo_ety = reg.eyReg(reg.Central) + reg.eyReg(reg.EndCap) + reg.eyReg(reg.Forward)
        newMET_LocHadTopo_etsum = reg.etSumReg(reg.Central) + reg.etSumReg(reg.EndCap) + reg.etSumReg(reg.Forward)
        if muon_algo == "Staco":
            midmet = "MET_MuonBoy"
        else:
            midmet = "MET_Muid"
        met.x = newMET_LocHadTopo_etx + self.sg[midmet].etx() - self.sg["MET_RefMuon_Track"].etx()
        met.y = newMET_LocHadTopo_ety + self.sg[midmet].ety() - self.sg["MET_RefMuon_Track"].ety()
        met.sum = newMET_LocHadTopo_etsum + self.sg[midmet].sumet() - self.sg["MET_RefMuon_Track"].sumet()

        met.met_central.x = reg.exReg(reg.Central); 
        met.met_central.y = reg.eyReg(reg.Central); 
        met.met_central.sum = reg.etSumReg(reg.Central); 
        met.met_endcap.x = reg.exReg(reg.EndCap); 
        met.met_endcap.y = reg.eyReg(reg.EndCap); 
        met.met_endcap.sum = reg.etSumReg(reg.EndCap); 
        met.met_forward.x = reg.exReg(reg.Forward); 
        met.met_forward.y = reg.eyReg(reg.Forward);
        met.met_forward.sum = reg.etSumReg(reg.Forward);
        return met

    def met_named(self, name):
        met = MissingEnergy()
        met.x = self.sg[name].etx();
        met.y = self.sg[name].ety();
        met.sum = self.sg[name].sumet();
        return met

    def met_detail(self, name):
        met = MissingEnergy()
        lht = self.sg[name]
        if not bool(lht):
            return met
        met.x = lht.etx()
        met.y = lht.ety()
        met.sum = lht.sumet()
        reg = lht.getRegions()
        if not bool(reg):
            return met
        met.met_central.x = reg.exReg(reg.Central); 
        met.met_central.y = reg.eyReg(reg.Central); 
        met.met_central.sum = reg.etSumReg(reg.Central); 
        met.met_endcap.x = reg.exReg(reg.EndCap); 
        met.met_endcap.y = reg.eyReg(reg.EndCap); 
        met.met_endcap.sum = reg.etSumReg(reg.EndCap); 
        met.met_forward.x = reg.exReg(reg.Forward); 
        met.met_forward.y = reg.eyReg(reg.Forward);
        met.met_forward.sum = reg.etSumReg(reg.Forward);
        return met

    def met_from_particles(self, particles):
        met = MissingEnergy()
        met.x = -sum(p.momentum().px() for p in particles)
        met.y = -sum(p.momentum().py() for p in particles)
        met.sum = sum(abs(p.momentum().perp()) for p in particles)
        return met

    def met_detail_from_particles(self, particles):
        met = self.met_from_particles(particles)
        # central: 1.5, endcap < 3.2; forward < 4.9
        met.met_central.CopyFrom(self.met_from_particles([p for p in particles if abs(p.momentum().eta()) <= 1.5]))
        met.met_endcap.CopyFrom(self.met_from_particles([p for p in particles if 1.5 < abs(p.momentum().eta()) <= 3.2]))
        met.met_forward.CopyFrom(self.met_from_particles([p for p in particles if 3.2 < abs(p.momentum().eta()) < 4.9]))
        return met

    def met(self):
        if self.year == 2010:
            return self.met_lochadtopo()
        else:
            return self.met_reffinal()

    def execute(self):
        event = Event()
        self.load_event_info(event) # sets run_number, event_number, lumi_block and mc_event_weight
        self.PV = list(self.sg["VxPrimaryCandidate"])
        self.PV_rec = [pv.recVertex() for pv in self.PV]
        self.metref_composition = self.sg["MET_RefComposition"]
        self.triggers(event.triggers)
        event.vertices.extend(self.vertices())

        event.met_LocHadTopo_modified.CopyFrom(self.met_lochadtopo())
        event.met_RefFinal45.CopyFrom(self.met_reffinal45())

        event.met_RefFinal.CopyFrom(self.met_detail("MET_RefFinal"))
        event.met_MuonBoy.CopyFrom(self.met_detail("MET_MuonBoy"))
        event.met_Muid.CopyFrom(self.met_detail("MET_Muid"))
        event.met_RefMuon_Track.CopyFrom(self.met_detail("MET_RefMuon_Track"))
        event.met_CellOut_em.CopyFrom(self.met_detail("MET_CellOut_em"))
        event.met_CellOut_Eflow.CopyFrom(self.met_detail("MET_CellOut_Eflow"))
        event.met_CellOut_Eflow_Muid.CopyFrom(self.met_detail("MET_CellOut_Eflow_Muid"))
        ### for DEV: needs to be fixed for CorrTopo & Final
        #event.met_CorrTopo.CopyFrom(self.met_detail("MET_CorrTopo"))
        #event.met_Final.CopyFrom(self.met_detail("MET_Final"))
        event.met_LocHadTopo.CopyFrom(self.met_detail("MET_LocHadTopo"))
        event.met_Muon.CopyFrom(self.met_detail("MET_Muon"))
        event.met_MuonMuid.CopyFrom(self.met_detail("MET_MuonMuid"))
        event.met_RefEle.CopyFrom(self.met_detail("MET_RefEle"))
        event.met_RefFinal_em.CopyFrom(self.met_detail("MET_RefFinal_em"))
        ### for dev testing STVF
        #event.met_RefFinal_STVF_em.CopyFrom(self.met_detail("MET_RefFinal_STVF_em"))
        #event.met_RefFinal_STVF.CopyFrom(self.met_detail("MET_RefFinal_STVF"))
        ### end dev
        event.met_RefGamma.CopyFrom(self.met_detail("MET_RefGamma"))
        event.met_RefJet.CopyFrom(self.met_detail("MET_RefJet"))
        event.met_RefMuon.CopyFrom(self.met_detail("MET_RefMuon"))
        event.met_RefMuon_Muid.CopyFrom(self.met_detail("MET_RefMuon_Muid"))
        event.met_RefMuon_Track_Muid.CopyFrom(self.met_detail("MET_RefMuon_Track_Muid"))
        event.met_RefTau.CopyFrom(self.met_detail("MET_RefTau"))
        event.met_SoftJets.CopyFrom(self.met_detail("MET_SoftJets"))
        event.met_Topo.CopyFrom(self.met_detail("MET_Topo"))
        event.met_Track.CopyFrom(self.met_detail("MET_Track"))
        if self.is_mc:
            event.met_Truth.CopyFrom(self.met_detail("MET_Truth"))
            event.met_Truth_PileUp.CopyFrom(self.met_detail("MET_Truth_PileUp"))
            #Nonint testing
            mettruth = self.sg["MET_Truth"]
            event.met_Truth_NonInt.x = mettruth.exTruth(mettruth.NonInt)
            event.met_Truth_NonInt.y = mettruth.eyTruth(mettruth.NonInt)

        event.jets_antikt4lctopo.extend(self.jets("AntiKt4LCTopoJets"))
        event.jets_antikt4h1topoem.extend(self.jets("AntiKt4TopoEMJets"))

        event.muons_staco.extend(self.muons("Staco"))
        event.muons_muid.extend(self.muons("Muid"))
        event.electrons.extend(self.electrons())
        #event.photons.extend(self.photons())
        self.tracks(event.tracks)

        if self.is_mc:
            if self.try_hfor:
                if not self.tool_hfor.execute().isFailure():
                    decision = self.tool_hfor.getDecision()
                    if decision == "":
                        decision = "None"
                    event.hfor_decision = getattr(event, decision)
                else:
                    self.try_hfor = False
                
            # extract truth info
            #hard_event = [x for x in get_all_particles(truth[0]) if is_final(x) and not is_coloured(x)]
            #pileup = sum(([x for x in get_all_particles(t) if is_final(x) and not is_coloured(x)] for t in truth[1:]), [])
            tmx, tmy, tms = 0, 0, 0
            ptmx, ptmy, ptms = 0, 0, 0

            truth = list(self.sg["GEN_AOD"])

            hard = True
            for t in truth:
                if hard:
                    if t.alphaQCD() != -1:
                        event.pdf_info.alpha_qcd = t.alphaQCD()
                    if t.alphaQED() != -1:
                        event.pdf_info.alpha_qed = t.alphaQED()
                    if t.event_scale() != -1:
                        event.pdf_info.event_scale = t.event_scale()
                    if t.mpi() != -1:
                        event.pdf_info.mpi = t.mpi()
                    pdfi = t.pdf_info()
                    event.pdf_info.id1, event.pdf_info.id2 = pdfi.id1(), pdfi.id2()
                    event.pdf_info.pdf1, event.pdf_info.pdf2 = pdfi.pdf1(), pdfi.pdf2()
                    event.pdf_info.scale_pdf = pdfi.scalePDF()
                    event.pdf_info.x1, event.pdf_info.x2 = pdfi.x1(), pdfi.x2()

                for p in get_all_particles(t):
                    if 10 < p.pdg_id() < 19:
                        if is_final(p) and not is_coloured(p) and p.momentum().perp() > 5000.0:
                            if p.pdg_id() == 11:
                                set_truth(event.truth_electrons.add(), p.momentum(), p.pdg_id()/11)
                            elif p.pdg_id() == 13:
                                set_truth(event.truth_muons.add(), p.momentum(), p.pdg_id()/13)
                            if p.pdg_id() in (12, 14, 16, 18):
                                set_lv(event.truth_neutrinos.add(), p.momentum())
                        #elif hard:
                        #    tmx -= p.momentum().px()
                        #    tmy -= p.momentum().py()
                        #    tms -= p.momentum().perp()
                        #else:
                        #    ptmx -= p.momentum().px()
                        #    ptmy -= p.momentum().py()
                        #    ptms -= p.momentum().perp()
                hard = False

            #event.truth_met_hard.x = tmx
            #event.truth_met_hard.y = tmy
            #event.truth_met_hard.sum = tms
            #event.truth_met_pileup.x = ptmx
            #event.truth_met_pileup.y = ptmy
            #event.truth_met_pileup.sum = ptms

            #event.truth_met_hard.CopyFrom(self.met_detail_from_particles([p for p in hard_event if not p.pdg_id() in (12, 14, 16, 18)]))
            #event.truth_met_pileup.CopyFrom(self.met_detail_from_particles([p for p in pileup if not p.pdg_id() in (12, 14, 16, 18)]))
            #event.truth_neutrinos.extend(make_lv(p.momentum()) for p in hard_event+pileup if p.pdg_id() in (12, 14, 16, 18))
            #event.truth_electrons.extend(make_truth(p.momentum(), p.pdg_id()/11) for p in hard_event+pileup if p.pdg_id() == 11)
            #event.truth_muons.extend(make_truth(p.momentum(), p.pdg_id()/13) for p in hard_event+pileup if p.pdg_id() == 13)

        self.a4.write(event)
        #self.event_counter += 1
        #print "TIGHT INFO at event ", self.event_counter , ": ", self.tight_same_count, self.only_othertight_count, self.only_tight_count

        return PyAthena.StatusCode.Success

def make_list(begin, end):
    result = []
    while begin.__cpp_ne__(end):
        result.append(begin.__deref__())
        begin.__preinc__()
    return result

def get_all_particles(ev):
    return make_list(ev.particles_begin(), ev.particles_end())

def get_particles_in(v):
    begin = v.particles_in_const_begin()
    end = v.particles_in_const_end()
    return make_list(begin, end)

def get_particles_out(v):
    begin = v.particles_out_const_begin()
    end = v.particles_out_const_end()
    return make_list(begin, end)

def is_coloured(p):
    pdgid = p.pdg_id()
    return abs(pdgid) < 10 or pdgid == 21 or (abs(pdgid) < 9999 and (abs(pdgid)/10)%10 == 0)

def is_final(p):
    return not bool(p.end_vertex())


if not "options" in dir():
    raise RuntimeError("No options set!")

# Set default values for testing during local running and setup athena
a_local_directory = "/data/etp"
if os.path.exists(a_local_directory):
    if "input" in options:
        input = glob(options["input"]) 
    else:
        input = glob("/data/etp/ebke/data/*109074*/*")
    athena_setup(input, -1)
else:
    athena_setup(None, -1)

# JVF fix by Scott Snyder
if True:
    from RecExConfig.RecFlags import rec
    rec.UserExecs = ["myjets()"]
    def myjets():
        from JetRec.JetMomentGetter import make_JetMomentGetter

        from JetMomentTools.SetupJetMomentTools import getJetVertexAssociationTool
        jvatool = getJetVertexAssociationTool('AntiKt', 0.4, 'Topo') # parameters are irrelevant, these will work for any jets
        #make_JetMomentGetter( 'AntiKt4TopoJets' , [jvatool] ) 
        make_JetMomentGetter( 'AntiKt4TopoEMJets' , [jvatool] ) 


# do autoconfiguration of input
include ("RecExCommon/RecExCommon_topOptions.py")

#### test of implementing missing et d3pdmaker
#from MissingETD3PDMaker.MissingETD3PDMakerFlags        import MissingETD3PDMakerFlags
#from MissingETD3PDMaker.MissingETD3PDObject            import *
#from MissingETD3PDMaker.MissingETD3PDTriggerBitsObject import *
#MissingETD3PDMakerFlags.doCellOutEflow=True
#MissingETD3PDMakerFlags.METDefaultJetCollectionSGKey = 'AntiKt4LCTopoJets'
#MissingETD3PDMakerFlags.METDefaultJetPrefix = "jet_AntiKt4LCTopo_MET_"
#MissingETD3PDMakerFlags.doTruth=True
#from RecExConfig.RecFlags                            import rec
#rec.doTruth()

###Truth object, can also be used for customized MET Truth objects
#MissingETTruthD3PDObject = make_SG_D3PDObject ('MissingEtTruth', MissingETD3PDMakerFlags.METTruthSGKey(), 'MET_Truth_', 'MissingETTruthD3PDObject')
#MissingETTruthD3PDObject.defineBlock (1, 'MET_Truth_NonInt', MissingETD3PDMaker.MissingETTruthNonIntFillerTool)
#MissingETTruthD3PDObject.defineBlock (1, 'MET_Truth_NonInt_Phi', MissingETD3PDMaker.MissingETTruthNonIntPhiFillerTool)
#MissingETTruthD3PDObject.defineBlock (1, 'MET_Truth_NonInt_Et', MissingETD3PDMaker.ScalarMissingETTruthNonIntFillerTool)
#MissingETTruthD3PDObject.defineBlock (1, 'MET_Truth_NonInt_SumEt', MissingETD3PDMaker.SumETTruthNonIntFillerTool)
#MissingETTruthD3PDObject.defineBlock (3, 'MET_Truth_Int', MissingETD3PDMaker.MissingETTruthIntFillerTool)
#MissingETTruthD3PDObject.defineBlock (3, 'MET_Truth_Int_Phi', MissingETD3PDMaker.MissingETTruthIntPhiFillerTool)
#MissingETTruthD3PDObject.defineBlock (3, 'MET_Truth_Int_Et', MissingETD3PDMaker.ScalarMissingETTruthIntFillerTool)
#MissingETTruthD3PDObject.defineBlock (3, 'MET_Truth_Int_SumEt', MissingETD3PDMaker.SumETTruthIntFillerTool)

#topSequence += MissingETTruthD3PDObject 
#from AthenaCommon.AlgSequence import AlgSequence
#theJob = AlgSequence()
#theJob += MissingETTruthD3PDObject(level=10,sgkey='MissingET', include=['MET_Truth_NonInt'], allowMissing=True)

ana = AOD2A4("AOD2A4", int(options["year"]), options)
topSequence += ana
