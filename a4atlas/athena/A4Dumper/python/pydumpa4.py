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
from a4.atlas.Event_pb2 import Event, Track
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
        "EF_e20_medium",
        "EF_e22_medium",
        "EF_e22vh_medium1",
        "EF_mu18_MG",
        "EF_mu18_MG_medium",
        "EF_mu20_MG",
        "EF_e10_medium_mu6",
        "EF_2mu10",
        "EF_2e12_medium",
        "EF_mu40_MSonly",
        "EF_mu40_MSonly_barrel",
        "EF_mu20_empty"
        ]
}

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

def make_vertex(vx):
    v = Vertex()
    v.x, v.y, v.z = vx.x(), vx.y(), vx.z()
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
        self.jet_fmax = lambda jet : JetCaloQualityUtils.fracSamplingMax(jet, Long(CaloSampling.Unknown))
        self.jet_time = lambda jet : JetCaloQualityUtils.jetTimeCells(jet)
        self.jet_quality_lar = lambda jet : JetCaloQualityUtils.jetQualityLAr(jet)
        self.jet_quality_hec = lambda jet : JetCaloQualityUtils.jetQualityHEC(jet)

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
        self.tool_tmt = PyAthena.py_tool("TrigMatchTool/TrigMatchTool")
        self.tool_hfor= PyAthena.py_tool("HforTool",iface="IHforTool")
 
    def tracks(self):
        trks = []
        for i, trk in enumerate(self.sg["TrackParticleCandidate"]):
            if abs(trk.pt()) < 1000:
                continue
            t = Track()
            t.p4.CopyFrom(make_lv(trk))
            t.charge = int(trk.charge())

            vx = trk.reconstructedVertex()
            if vx:
                t.vertex_index = list(self.sg["VxPrimaryCandidate"]).index(vx)
            t.hits.CopyFrom(make_track_hits(trk))
            t.perigee.CopyFrom(self.perigee_z0_d0(trk))
            trks.append(t)
        return trks

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
                e.vertex_index = list(vc.recVertex() for vc in self.sg["VxPrimaryCandidate"]).index(vx)
            e.author = el.author()
            for iso in isolations:
                setattr(e.isolation, iso, el.detailValue(getattr(self.egammaParameters, iso)))

            if self.year == 2011:
                e.bad_oq = not (el.isgoodoq(self.egammaPID.BADCLUSELECTRON) == 0)
            elif self.year == 2010:
                e.bad_oq = not (self.egOQ.checkOQClusterElectron(167521, el.cluster.Eta(), el.cluster.Phi()) != 3)

            e.loose = bool(el.isElectron(self.egammaPID.ElectronLoose))
            e.medium = bool(el.isElectron(self.egammaPID.ElectronMedium))
            if self.year == 2010:
                e.tight = bool(el.isElectron(self.egammaPID.ElectronTight_WithTrackMatch))
            else:
                e.tight = bool(el.isElectron(self.egammaPID.ElectronTight))

            trk = el.trackParticle()
            if trk:
                e.p4_track.CopyFrom(make_lv(trk))
                e.perigee.CopyFrom(self.perigee_z0_d0(trk))
                e.track_hits.CopyFrom(make_track_hits(trk))
            if el.cluster():
                e.p4_cluster.CopyFrom(make_lv(el.cluster()))

            if trk:
                for chain in list(self.tool_tmt.__getattribute__("chainsPassedByObject<TrigElectron, INavigable4Momentum>")(trk, 0.15)):
                    if chain in trigger_names[self.year]:
                        e.matched_trigger.append(getattr(Trigger,chain))

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
            vx = mu.origin()
            if vx:
                #if vx.position():
                #    m.vertex.CopyFrom(make_vertex(vx.position()))
                m.vertex_index = list(vc.recVertex() for vc in self.sg["VxPrimaryCandidate"]).index(vx)
            for iso in isolations:
                setattr(m.isolation, iso, mu.parameter(getattr(self.MuonParameters, iso)))

            m.tight = (mu.isTight() == 1)
            if muon_algo == "Muid":
                m.combined = mu.isAuthor(self.MuonParameters.MuidCo)
            elif muon_algo == "Staco":
                m.combined = mu.isAuthor(self.MuonParameters.STACO) and mu.isCombinedMuon()

            trk = mu.inDetTrackParticle()
            if trk:
                m.p4_track.CopyFrom(make_lv(trk))
                m.perigee_id.CopyFrom(self.perigee_z0_d0(trk))
                m.track_hits.CopyFrom(make_track_hits(trk))
    
            ms_trk = mu.muonExtrapolatedTrackParticle()
            if ms_trk:
                m.p4_ms.CopyFrom(make_lv(ms_trk))

            ctrk = mu.combinedMuonTrackParticle()
            if ctrk:
                m.perigee_cmb.CopyFrom(self.perigee_z0_d0(trk))
                m.ms_hits.CopyFrom(make_ms_track_hits(ctrk))

            m.matched_trigger_efi_ms.extend(self.matched_chains(mu, useSpectrometerTrack))
            m.matched_trigger_efi_ex.extend(self.matched_chains(mu, useExtrapolatedTrack))
            m.matched_trigger_efi_cb.extend(self.matched_chains(mu, useCombinedTrack))
            m.matched_trigger_efi_mg.extend(self.matched_chains(mu, useMuGirlTrack))
            #A17 m.matched_trigger_efi_mgt.extend(self.matched_chains(mu, useMuGirlTagTrack))

            for chain in list(self.tool_tmt.__getattribute__("chainsPassedByObject<CombinedMuonFeature, INavigable4Momentum>")(mu,0.1)):
                if chain in trigger_names[self.year]:
                    m.matched_trigger_cmf.append(getattr(Trigger,chain))
            for chain in list(self.tool_tmt.__getattribute__("chainsPassedByObject<TrigMuonEF, INavigable4Momentum>")(mu,0.1)):
                if chain in trigger_names[self.year]:
                    m.matched_trigger_ef.append(getattr(Trigger,chain))
            for chain in list(self.tool_tmt.__getattribute__("chainsPassedByObject<MuonFeature, INavigable4Momentum>")(mu,0.1)):
                if chain in trigger_names[self.year]:
                    m.matched_trigger_mf.append(getattr(Trigger,chain))

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

            j.p4_em.CopyFrom(make_lv(jet.hlv(JETEMSCALE)))
            j.EMJES = jet.getMoment("EMJES")
            j.SV0 = jet.getFlavourTagWeight("SV0")
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
            if jet.pt() > 20*GeV:
                j.lar_quality = jet.getMoment("LArQuality")
                j.hec_quality = jet.getMoment("HECQuality")
                j.negative_e = jet.getMoment("NegativeE")
                j.emf = self.jet_emf(jet)
                j.hecf = self.jet_hecF(jet)
                j.timing = jet.getMoment("Timing")
                j.fmax = self.jet_fmax(jet)
                j.sum_pt_trk = jet.getMoment("sumPtTrk")

            jets.append(j)
        return jets

    def matched_chains(self, mu, which_track):
        self.tmefih.setTrackToUse(which_track)
        chains = []
        for chain in list(self.tool_tmt.__getattribute__("chainsPassedByObject<TrigMuonEFInfo, INavigable4Momentum>")(mu, 0.1)):
            if chain in trigger_names[self.year]:
                chains.append(getattr(Trigger, chain))
        return chains
        
    def triggers(self):
        triggers = []
        self.tmei = []
        for tn in trigger_names[self.year]:
            t = Trigger()
            t.name = getattr(t, tn)
            t.fired = self.tool_tdt.isPassed(tn)
            if t.fired:
                te  = list(self.tool_tmt.__getattribute__("getTriggerObjects<TrigElectron>")(tn, True))
                tp  = list(self.tool_tmt.__getattribute__("getTriggerObjects<TrigPhoton>")(tn, True))
                tme = list(self.tool_tmt.__getattribute__("getTriggerObjects<TrigMuonEF>")(tn, True))
                mf  = list(self.tool_tmt.__getattribute__("getTriggerObjects<MuonFeature>")(tn, True))
                cmf = list(self.tool_tmt.__getattribute__("getTriggerObjects<CombinedMuonFeature>")(tn, True))
                tmei= list(self.tool_tmt.__getattribute__("getTriggerObjects<TrigMuonEFInfo>")(tn, True))

                tmeit = sum((list(efi.TrackContainer()) for efi in tmei), [])
                tmeit_ms = [tr.SpectrometerTrack() for tr in tmeit if tr.MuonType() == 1 and tr.hasSpectrometerTrack()]
                tmeit_ex = [tr.ExtrapolatedTrack() for tr in tmeit if tr.MuonType() == 1 and tr.hasExtrapolatedTrack()]
                tmeit_cb = [tr.CombinedTrack() for tr in tmeit if tr.MuonType() == 1 and tr.hasCombinedTrack()]
                tmeit_mg = [tr.CombinedTrack() for tr in tmeit if tr.MuonType() == 2 and tr.hasCombinedTrack()]

                def make_tf(feature):
                    ff = TriggerFeature()
                    ff.eta = feature.eta()
                    ff.phi = feature.phi()
                    ff.pt = feature.pt()
                    return ff

                t.features_trig_electron.extend(make_tf(f) for f in te)
                t.features_trig_photon.extend(make_tf(f) for f in tp)
                t.features_trig_muon_ef.extend(make_tf(f) for f in tme)
                t.features_trig_muon_efi_ms.extend(make_tf(f) for f in tmeit_ms)
                t.features_trig_muon_efi_ex.extend(make_tf(f) for f in tmeit_ex)
                t.features_trig_muon_efi_cb.extend(make_tf(f) for f in tmeit_cb)
                t.features_trig_muon_efi_mg.extend(make_tf(f) for f in tmeit_mg)
                t.features_muon.extend(make_tf(f) for f in mf)
                t.features_muon_combined.extend(make_tf(f) for f in cmf)
 
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
            p = Perigee()
            vxp = self.sg["VxPrimaryCandidate"][0].recVertex().position()
            pavV0 = self.tool_ttv.perigeeAtVertex(trk, vxp)
            p.d0 = pavV0.parameters()[0]
            p.d0err = pavV0.localErrorMatrix().error(0)
            p.z0 = pavV0.parameters()[1]
            p.z0err = pavV0.localErrorMatrix().error(0)
            return p
        return None

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
        gc.collect(2)
        event = Event()
        self.load_event_info(event) # sets run_number, event_number, lumi_block and mc_event_weight
        event.triggers.extend(self.triggers())
        event.vertices.extend(self.vertices())

        event.met_LocHadTopo_modified.CopyFrom(self.met_lochadtopo())
        event.met_RefFinal45.CopyFrom(self.met_reffinal45())

        event.met_RefFinal.CopyFrom(self.met_detail("MET_RefFinal"))
        event.met_MuonBoy.CopyFrom(self.met_detail("MET_MuonBoy"))
        event.met_Muid.CopyFrom(self.met_detail("MET_Muid"))
        event.met_RefMuon_Track.CopyFrom(self.met_detail("MET_RefMuon_Track"))
        event.met_CorrTopo.CopyFrom(self.met_detail("MET_CorrTopo"))
        event.met_Final.CopyFrom(self.met_detail("MET_Final"))
        event.met_LocHadTopo.CopyFrom(self.met_detail("MET_LocHadTopo"))
        event.met_Muon.CopyFrom(self.met_detail("MET_Muon"))
        event.met_MuonMuid.CopyFrom(self.met_detail("MET_MuonMuid"))
        event.met_RefEle.CopyFrom(self.met_detail("MET_RefEle"))
        event.met_RefFinal_em.CopyFrom(self.met_detail("MET_RefFinal_em"))
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

        #event.jets_antikt4lctopo.extend(self.jets("AntiKt4LCTopoJets"))
        event.jets_antikt4h1topoem.extend(self.jets("AntiKt4TopoEMJets"))

        event.muons_staco.extend(self.muons("Staco"))
        event.muons_muid.extend(self.muons("Muid"))
        event.electrons.extend(self.electrons())
        #event.photons.extend(self.photons())
        event.tracks.extend(self.tracks())

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
           
            truth = list(self.sg["GEN_AOD"])
            hard_event = [x for x in get_all_particles(truth[0]) if is_final(x) and not is_coloured(x)]
            pileup = sum(([x for x in get_all_particles(t) if is_final(x) and not is_coloured(x)] for t in truth[1:]), [])

            event.truth_met_hard.CopyFrom(self.met_detail_from_particles([p for p in hard_event if not p.pdg_id() in (12, 14, 16, 18)]))
            event.truth_met_pileup.CopyFrom(self.met_detail_from_particles([p for p in pileup if not p.pdg_id() in (12, 14, 16, 18)]))
            event.neutrinos.extend(make_lv(p.momentum()) for p in hard_event+pileup if p.pdg_id() in (12, 14, 16, 18))

        self.a4.write(event)
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
    athena_setup(input, 100)
else:
    athena_setup(None, -1)

# JVF fix by Scott Snyder
if False:
    from RecExConfig.RecFlags import rec
    rec.UserExecs = ["myjets()"]
    def myjets():
        from JetRec.JetMomentGetter import make_JetMomentGetter

        from JetMomentTools.SetupJetMomentTools import getJetVertexAssociationTool
        jvatool = getJetVertexAssociationTool('AntiKt', 0.4, 'Topo') # parameters are irrelevant, these will work for any jets
        make_JetMomentGetter( 'AntiKt4TopoJets' , [jvatool] ) 
        make_JetMomentGetter( 'AntiKt4TopoEMJets' , [jvatool] ) 

# do autoconfiguration of input
include ("RecExCommon/RecExCommon_topOptions.py")

ana = AOD2A4("AOD2A4", int(options["year"]), options)
topSequence += ana
