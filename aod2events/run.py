#! /usr/bin/env python

# notes: el.cluster().pt() == el.cluster().et()
#       def cluster_pt(el):
#               return el.cluster().e()*sin(theta_from_eta(el.cluster().eta))

# Scripts expect to be run with a cwd where "minty" and "pytuple" are present.
# (Or minty and pytuple are in the path)

import os; import sys; sys.path.insert(0, os.getcwd())
from glob import glob
from bisect import bisect
from cPickle import load
from math import sin, atan, atan2, exp, pi

from aod2a4 import AOD2A4, athena_setup

from AthenaCommon.AppMgr import topSequence
from ROOT import gROOT, TTree, AddressOf, TLorentzVector

from TreeHelper import TreeHelper, getMomentumLV

from array import array


def FEB_pass_jet(hlv):
    eta, phi = hlv.eta(), hlv.phi()
    #return not ((eta < 1.85 and eta > -0.4) and (phi < -0.192 and phi > -1.188))
    return not ((-0.4 < eta < 1.85) and (-1.188 < phi < -0.192))

def FEB_pass_el(eta, phi):
    #return not ((eta < 1.55 and eta > -0.1) and (phi < -0.492 and phi > -0.888))
    return not ((-0.1 < eta < 1.55) and (-0.888 < phi < -0.492))


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


MZ = 91.1876*GeV
JETEMSCALE = 0 # http://alxr.usatlas.bnl.gov/lxr/source/atlas/Event/EventKernel/EventKernel/ISignalState.h#021

def dphi(phi1, phi2):
    dphi = abs(phi1 - phi2)
    return 2*pi - dphi if dphi > pi else dphi

# Set default values for testing during local running and setup athena
a_local_directory = "/data/etp"
if os.path.exists(a_local_directory):
    if "input" in options:
        input = glob(options["input"]) 
    else:
        input = glob("/data/etp/ebke/data/*109074*/*")
    athena_setup(input, -1)
    skim_type = "AOD"
else:
    athena_setup(None, -1)
    skim_type = "AOD"

if not "options" in dir():
    raise RuntimeError("No options set!")

# do autoconfiguration of input
include ("RecExCommon/RecExCommon_topOptions.py")

def or_func(c1, c1g, c2g, delta):
    return lambda e : [x for x in getattr(e, c1)
                       if min([100] + [c1g(x).DeltaR(y) for y in c2g(e)]) > delta]

def selfoverlap_func(c1, c1g, delta):
    return lambda e : [x for x in getattr(e, c1)
                       if min([100] + [c1g(x).DeltaR(c1g(y)) for y in getattr(e, c1) if x != y]) > delta]

def cached_property(f):
    #return property(event_cache(f))
    return property(f)

class AnalysisObject(object):
    def __init__(self, lv, charge, index=-1):
        self.lv = lv # primary TLorentzVector. The Object can define additional ones
        self.charge = charge
        self.index = index
        self.met_contribution = TLorentzVector()

    def scale_to(self, et):
        lv_old = self.lv
        self.lv = self.lv * (et/self.pt)
        self.met_contribution += lv_old - self.lv

    def scale_factor(self, factor):
        lv_old = self.lv
        self.lv = self.lv * factor
        self.met_contribution += lv_old - self.lv

    @property
    def eta(self):
        return self.lv.Eta()

    @property
    def phi(self):
        return self.lv.Phi()

    @property
    def pt(self):
        return self.lv.Pt()

    @property
    def e(self):
        return self.lv.E()


class EAnalysisElectron(AnalysisObject):
    class_data = None
    @classmethod
    def init(cls):
        import PyCintex
        PyCintex.loadDictionary('egammaEnumsDict')
        from ROOT import egammaParameters, egammaPID
        EAnalysisElectron.egammaParameters = egammaParameters
        EAnalysisElectron.egammaPID = egammaPID

        gROOT.ProcessLine(".L EnergyRescaler.C+") # includes all of the above
        from ROOT import EnergyRescaler
        cls.el_rescale = EnergyRescaler()
        cls.el_rescale.useDefaultCalibConstants("2011")

        from ROOT import egammaSFclass
        EAnalysisElectron.egammaSF = egammaSFclass()

        EAnalysisElectron.class_data = True

    def __init__(self, el, index):
        if EAnalysisElectron.class_data is None:
            EAnalysisElectron.init()
        self.el = el
        if el.trackParticle():
            super(EAnalysisElectron, self).__init__(getMomentumLV(el.trackParticle()), el.charge(), index)
        else: # oh, a photon
            super(EAnalysisElectron, self).__init__(getMomentumLV(el), el.charge(), index)
        self.id = TLorentzVector(self.lv)
        if el.cluster() is None:
            self.cluster = TLorentzVector()
        else:
            self.cluster = getMomentumLV(el.cluster())
            sfac = self.lv.E()/self.cluster.E()
            self.lv *= 1.0/sfac
            #self.met_contribution += self.lv * (1. - (1./sfac))

    @cached_property
    def cluster_eta(self):
        return self.cluster.Eta()

    @cached_property
    def cluster_pt(self):
        return self.cluster.Pt()

    def scale_to(self, et):
        factor = et/self.pt
        self.id *= factor
        self.cluster *= factor
        rv = super(EAnalysisElectron, self).scale_factor(factor)
        return rv

    def author(self, auth):
        return self.el.author() == auth

    @property
    def in_crack_region(self):
        return 1.37 <= abs(self.cluster_eta) <= 1.52

    def pass_otx(self, run_number, is_mc):
        #rn = 167521 if is_mc else run_number # last pp run
        # always use last pp run according to https://twiki.cern.ch/twiki/bin/view/AtlasProtected/OQMapsUsage
        rn = 167521
        return EAnalysisElectron.egOQ.checkOQClusterElectron(rn, self.cluster.Eta(), self.cluster.Phi()) != 3

    @cached_property
    def inner_track(self):
        return self.el.trackParticle()
    track = inner_track

    def good_track_hits(self):
        track = self.inner_track
        if not track:
            return False
        summary = track.trackSummary()
        if not summary:
            return False
        if summary.get(SummaryType.numberOfPixelHits) + summary.get(SummaryType.numberOfSCTHits) >= 4:
            return True
        return False

    @property
    def higgs_lv(self):
        if self.good_track_hits():
            return self.lv
        return self.cluster

    @property
    def higgs_pt(self):
        if self.good_track_hits():
            return self.lv.Pt()
        return self.cluster_pt

    @property
    def in_feb(self):
        return not FEB_pass_el(self.cluster.Eta(), self.cluster.Phi())

class EAnalysisMuon(AnalysisObject):
    class_data = None
    @classmethod
    def init(cls):
        import PyCintex
        PyCintex.loadDictionary("muonEventDict")
        PyCintex.loadDictionary("MuonEfficiencyCorrections")
        cls.eff_sf_staco_tight_cls = PyCintex.makeClass("Analysis::StacoTightScaleEffFactors")
        cls.eff_sf_muid_tight_cls = PyCintex.makeClass("Analysis::MuidTightScaleEffFactors")
        cls.eff_sf_staco_cls = PyCintex.makeClass("Analysis::StacoCBScaleEffFactors")
        cls.eff_sf_muid_cls = PyCintex.makeClass("Analysis::MuidCBScaleEffFactors")
        cls.eff_sf_staco = cls.eff_sf_staco_cls()
        cls.eff_sf_muid = cls.eff_sf_muid_cls()
        cls.eff_sf_staco_tight = cls.eff_sf_staco_tight_cls()
        cls.eff_sf_muid_tight = cls.eff_sf_muid_tight_cls()

        gROOT.ProcessLine(".L MuonSmearingClass.C++") # includes all of the above
        from ROOT import SmearingClass
        cls.muon_smearing = SmearingClass()
        cls.muon_smearing.UseScale(1)

        from ROOT import MuonParameters
        EAnalysisMuon.MuonParameters = MuonParameters
        EAnalysisMuon.class_data = True

    def __init__(self, mu, index):
        if EAnalysisMuon.class_data is None:
            EAnalysisMuon.init()
        self.mu = mu
        super(EAnalysisMuon, self).__init__(getMomentumLV(mu), mu.charge(), index)
        if mu.inDetTrackParticle():
            self.id = getMomentumLV(mu.inDetTrackParticle())
        else:
            self.id = TLorentzVector() 
        if mu.muonSpectrometerTrackParticle():
            self.ms = getMomentumLV(mu.muonSpectrometerTrackParticle())
            self.ms_ex = getMomentumLV(mu.muonExtrapolatedTrackParticle())
        else:
            self.ms = TLorentzVector() 
            self.ms_ex = TLorentzVector() 

    @property
    def tight(self):
        return self.mu.isTight() == 1

    @cached_property
    def muid_combined(self):
        return self.mu.isAuthor(EAnalysisMuon.MuonParameters.MuidCo)

    @cached_property
    def staco_combined(self):
        return self.mu.isAuthor(EAnalysisMuon.MuonParameters.STACO) and self.mu.isCombinedMuon() == 1

    @cached_property
    def combined(self):
        return self.muid_combined or self.staco_combined

    @cached_property
    def inner_track(self):
        return self.mu.inDetTrackParticle()

    @cached_property
    def track(self):
        return self.mu.combinedMuonTrackParticle()

    def scale_to(self, MS, ID, CB):
        if self.ms.Pt() != 0:
            self.ms *= MS/self.ms.Pt()
            self.ms_ex *= MS/self.ms.Pt()
        if self.id.Pt() != 0:
            self.id *= ID/self.id.Pt()
        return super(EAnalysisMuon, self).scale_to(CB)

    def scale_factor(self, factor):
        self.ms *= factor
        self.ms_ex *= factor
        self.id *= factor
        return super(EAnalysisMuon, self).scale_factor(factor)

    @property
    def good_trt(self):
        hits = self.mu.numberOfTRTHits()
        outliers = self.mu.numberOfTRTOutliers()
        n = hits + outliers
        if abs(self.eta) < 1.9:
            return n > 5 and outliers < 0.9 * n
        else:
            return n <= 5 or outliers < 0.9 * n


class JEAnalysis(AnalysisAlgorithm):

    @cached_property
    def jets(self):
        return self.sg["AntiKt4TopoEMJets"]

    @cached_property
    def els(self):
        aod_els = self.sg["ElectronAODCollection"]
        els = [EAnalysisElectron(el, i) for i, el in enumerate(aod_els)]
        if self.correct_electrons:
            if self.year == 2010:
                for el in els:
                    et = self.corrections.ElecEtCorr(el.eta, el.pt/GeV, el.e/GeV, self.run_number, self.event_number, el.index)
                    el.scale_to(et*GeV)
            elif self.year == 2011:
                for el in els:
                    eRescale = EAnalysisElectron.el_rescale
                    eRescale.SetRandomSeed(1771561 + self.event_number + (el.index * 10))

                    e_cl = el.cluster.E()
                    eta_cl = el.cluster.Eta()
                    e_new = e_cl

                    if not self.is_mc:
                        # scale correction
                        #e_new = GeV*eRescale.applyEnergyCorrection(eta_cl, el.cluster.Phi(), e_cl/GeV, e_cl/cosh(el.lv.Eta())/GeV, 0, "ELECTRON");
                        pass
                    else:
                        # scale uncertainties
                        #eRescale.getError(eta_cl, e_cl/cosh(eta_trk)/GeV, er_up, er_do, "ELECTRON");
                        #smear = (1 + er_up)
                        #smear = (1 + er_do)

                        # resolution correction
                        smear = eRescale.getSmearingCorrection(eta_cl, e_cl/GeV, 0, True)
                        # resolution uncertainties
                        #smear = eRescale.getSmearingCorrection(eta_cl, e_cl/GeV, 2, True) #up
                        #e_new = e_cl*smearcorr_up;
                        #smear = eRescale.getSmearingCorrection(eta_cl, e_cl/GeV, 1, True) #down
                        el.scale_factor(smear) 

        if self.correct_electron_efficiency == "Tight":
            assert self.year == 2011
            for el in els:
                p = EAnalysisElectron.scaleFactorTight(el.eta, el.cluster_et, 0, 2, True)
                eff, unc = p.first, p.second
                el.scale_factor = eff

        elif not self.correct_electron_efficiency is None:
            raise Exception("Unknown correct_electron_efficiency: %s" % self.correct_electron_efficiency)

        return els

    @cached_property
    def mus(self):
        aod_mus = self.sg["%sMuonCollection" % self.muon_algo]
        mus = [EAnalysisMuon(mu, i) for i, mu in enumerate(aod_mus)]
        if self.smear_muons and self.is_mc:
            for i, mu in enumerate(mus):
                EAnalysisMuon.muon_smearing.SetSeed(self.event_number, i)
                EAnalysisMuon.muon_smearing.Event(mu.ms.Pt(), mu.id.Pt(), mu.pt, mu.lv.Eta())
                # Get Smeared Pts
                pTCB_smeared = EAnalysisMuon.muon_smearing.pTCB()
                pTMS_smeared = EAnalysisMuon.muon_smearing.pTMS()
                pTID_smeared = EAnalysisMuon.muon_smearing.pTID() 
                # print "scaled from", mu.pt , " to ", pTCB_smeared
                # Systematics:
                # EAnalysisMuon.PTVar(pTMS_smeared, pTID_smeared, pTCB_smeared, THESTRING);
                # Valid values for "THESTRING": {"MSLOW", "MSUP", "IDLOW", "IDUP"} 
                mu.scale_to(pTMS_smeared, pTID_smeared, pTCB_smeared)
        return mus

    def vx_id_d0_err(self, lep):
        if len(self.vertices) == 0 or not lep.inner_track:
            raise CutNotApplicable
        vxp = self.vertices[0].recVertex().position()
        pavV0 = self.tool_ttv.perigeeAtVertex(lep.inner_track, vxp)
        return pavV0.parameters()[0]/pavV0.localErrorMatrix().error(0)

    def vx_id_z0(self,lep):
        if len(self.vertices) == 0 or not lep.inner_track:
            raise CutNotApplicable
        vxp = self.vertices[0].recVertex().position()
        pavV0 = self.tool_ttv.perigeeAtVertex(lep.inner_track, vxp)
        return pavV0.parameters()[1]

    def __init__(self, name, options, hm):
        super(JEAnalysis, self).__init__(name, options, hm)
        if self.year == 2010:
            gROOT.ProcessLine(".L RndmMuonSmearing.C++") # includes all of the above
            gROOT.ProcessLine(".L RndmFromEvtID.C++") # includes all of the above
            from ROOT import RndmFromEvtID
            self.corrections = RndmFromEvtID(False)

    def initialize_trigger(self):
        # acquire trigger tool
        self.tool_tdt = PyAthena.py_tool('Trig::TrigDecisionTool/TrigDecisionTool')

    def passed_triggers(self, channel):
        assert channel in ("mumu", "ee", "emu")
        passed = self.tool_tdt.isPassed
        if self.year == 2010:
            tl = ("L1_EM14", "EF_e15_medium", "L1_MU10", "EF_mu10_MG", "EF_mu13_MG", "EF_mu13_MG_tight")
            trig = map(lambda t : bool(passed(t)), tl)
            trig += [self.run_number, self.event_number]
            passed_el = self.corrections.IsPassElecTrigV(*trig)#, self.run_number, self.event_number)
            passed_mu = self.corrections.IsPassMuonTrigV(*trig)#, self.run_number, self.event_number)
            if channel == "emu":
                return passed_el or passed_mu
            elif channel == "mumu":
                return passed_mu
            elif channel == "ee":
                return passed_el
        elif self.year == 2011:
            if self.mu_trigger == "higgs":
                el = "EF_e20_medium"
                mu = ("EF_mu20_MG", )
            elif self.mu_trigger == "ww":
                el = "EF_e20_medium"
                mu = ("EF_mu18_MG", )
                #mu = ("EF_mu20", "EF_mu20_MG")
            if channel == "emu":
                if self.use_dilepton_triggers:
                    if passed("EF_e10_medium_mu6"):
                        return True
                return passed(el) or any(passed(m) for m in mu)
            elif channel == "mumu":
                if self.use_dilepton_triggers:
                    if passed("EF_2mu10"):
                        return True
                return any(passed(m) for m in mu)
            elif channel == "ee":
                if self.use_dilepton_triggers:
                    if passed("EF_2e12_medium"):
                        return True
                return passed(el)

    def initialize_stream_overlap(self):
        # read overlap files
        self.overlap_cache = {}
        pickles = [f for f in os.listdir(".") if f.startswith("overlap") and f.endswith(".pickle")]
        for pickle in pickles:
            self.overlap_cache.update(load(open(pickle)))
        print "--------- STREAM OVERLAP REMOVAL INFO ---------"
        print "read %i files:" % len(pickles)
        for pickle in pickles:
            print " * %s" % pickle
        print "list of runs:"
        print " ".join("%i" % run for run in sorted(self.overlap_cache.keys()))
        print "overlap per run:"
        for run in sorted(self.overlap_cache.keys()):
            print "%s : %i" % (run, len(self.overlap_cache[run]))
        print "--------- END STREAM OVERLAP REMOVAL INFO ---------"

    @cached_property
    def passed_grl(event):
        return event.is_mc or ((event.run_number, event.lumi_block) in event.grl)

    @cached_property
    def is_overlap(self):
        if self.is_mc:
            return False
        run, event = self.run_number, self.event_number
        if not int(run) in self.overlap_cache:
            return False
        elif int(event) in self.overlap_cache[run]:
            return True
        else:
            return False

    @cached_property
    def passed_d3pd(e):
        mus =  [mu for mu in e.sg["StacoMuonCollection"] if any(mu.isAuthor(i) for i in (5,6,7))]
        mus += [mu for mu in e.sg["MuidMuonCollection"] if any(mu.isAuthor(i) for i in (11,12,13,18))]
        for mu in mus:
            if mu.pt() > 10*GeV and abs(mu.eta()) <  2.7:
                return True
        for el in e.els:
            if el.pt > 10*GeV and abs(el.eta) <  2.7:
                return True
        return False

    @cached_property
    def met(self):
        if self.year == 2010 or self.met_loc_had_topo:
            lht = self.sg["MET_LocHadTopo"]
            reg = lht.getRegions()
            newMET_LocHadTopo_etx = reg.exReg(reg.Central) + reg.exReg(reg.EndCap) + reg.exReg(reg.Forward)
            newMET_LocHadTopo_ety = reg.eyReg(reg.Central) + reg.eyReg(reg.EndCap) + reg.eyReg(reg.Forward)
            if self.muon_algo == "Staco":
                midmet = "MET_MuonBoy"
            else:
                midmet = "MET_Muid"
            self.metX = newMET_LocHadTopo_etx + self.sg[midmet].etx() - self.sg["MET_RefMuon_Track"].etx()
            self.metY = newMET_LocHadTopo_ety + self.sg[midmet].ety() - self.sg["MET_RefMuon_Track"].ety()
        else:
            lht = self.sg["MET_RefFinal"]
            self.metX = lht.etx()
            self.metY = lht.ety()

        # Add lepton contribution
        lepton_contribution = TLorentzVector()
        for lep in self.selected_els + self.selected_mus:
            lepton_contribution += lep.met_contribution
        self.metX += lepton_contribution.X()
        self.metY += lepton_contribution.Y()

        return self.metX, self.metY

    @cached_property
    def met_proj(self):
        #If DeltaPhi < pi/2 then MET_project = MET*sin(DeltaPhi), else MET_project = MET, where
        #DeltaPhi = angle of MET and closest object (lepton, jet) 
        delta_phis = [dphi(o.phi, self.met_phi) for o in self.leptons]
        #TODO: only for 0j analysis delta_phis.extend(dphi(o.hlv(JETEMSCALE).phi(), self.met_phi) for o in self.selected_jets)
        delta_phi = min(delta_phis)
        if delta_phi < pi/2:
            return self.met_et * sin(delta_phi)
        else:
            return self.met_et
    
    @cached_property
    def met_et(self):
        x, y = self.met
        return (x**2 + y**2)**0.5

    @cached_property
    def met_phi(self):
        x, y = self.met
        return atan2(y, x)

    @cached_property
    def vertices(self):
        return self.sg["VxPrimaryCandidate"]

    def good_vertex(self, vx, n=3):
        return len(vx.vxTrackAtVertex()) >= n

    @cached_property
    def good_vertices(self, n=3):
        return [vx for vx in self.vertices if self.good_vertex(vx, n)]

    def initialize_jets(self):
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

    #@event_cache
    def is_cleaning_jet(self, jet):
        return jet.pt() > 20*GeV and (self.jet_bad(jet) or self.jet_ugly(jet))

    @cached_property
    def leptons(event):
        return list(reversed(sorted(event.selected_mus + event.selected_els)))

    @cached_property
    def ll(event):
        if len(event.leptons) >= 2:
            l1, l2 = event.leptons[:2]
            return l1.lv + l2.lv

    #@cached_property
    def get_perigee(self, l, id_only=False):
        tp = l.inner_track if id_only else l.track
        if len(self.vertices) > 0:
            v0 = self.vertices[0].recVertex().position()
            return self.tool_ttv.perigeeAtVertex(tp, v0) 
        else:
            return self.tool_ttv.perigeeAtVertex(tp) 

    def get_d0(self, l):
        return self.get_perigee(l, False).parameters()[0]

    def get_d0_id(self, l):
        return self.get_perigee(l, True).parameters()[0]

    def get_z0(self, l):
        return self.get_perigee(l, False).parameters()[1]
    
    def get_z0_id(self, l):
        return self.get_perigee(l, True).parameters()[1]

    @cached_property
    def l1(self):
        return self.leptons[0]

    @cached_property
    def l2(self):
        return self.leptons[1]

    def initialize_ntuple(self, name="ntuple"):
        self.ntup = TreeHelper(name, name, hsvc = self.histogram_manager)
        self.ntup.intBranch("electron_trigger")
        self.ntup.intBranch("muon_trigger")
        self.ntup.intBranch("event_number")
        self.ntup.intBranch("run_number")
        self.ntup.floatBranch("event_weight")
        self.ntup.intBranch("vx0_good")
        self.ntup.intBranch("vx_good_number")

        self.ntup.lvVBranch("muon")
        self.ntup.intVBranch("muon_charge")
        self.ntup.intVBranch("muon_index")
        self.ntup.lvVBranch("muon_id")
        self.ntup.lvVBranch("muon_ms")
        self.ntup.lvVBranch("muon_ms_ex")

        self.ntup.lvVBranch("electron")
        self.ntup.intVBranch("electron_charge")
        self.ntup.intVBranch("electron_index")
        self.ntup.lvVBranch("electron_cluster")
        self.ntup.lvVBranch("electron_track")

        self.ntup.lvVBranch("lepton")
        self.ntup.intVBranch("lepton_charge")
        self.ntup.lvBranch("ll")

        self.ntup.lvVBranch("jet")

        self.ntup.floatBranch("met_x")
        self.ntup.floatBranch("met_y")
        self.ntup.floatBranch("met_phi")
        self.ntup.floatBranch("met_proj")


    def fill_ntuple(self, event):
        self.ntup.execute()
        for mu in event.selected_mus:
            self.ntup.bPush("muon", mu.lv)
            self.ntup.bPush("muon_charge", int(mu.charge))
            self.ntup.bPush("muon_index", int(mu.index))
            self.ntup.bPush("muon_id", mu.id)
            self.ntup.bPush("muon_ms", mu.ms)
            self.ntup.bPush("muon_ms_ex", mu.ms_ex)
        for el in event.selected_els:
            self.ntup.bPush("electron", el.lv)
            self.ntup.bPush("electron_charge", int(el.charge))
            self.ntup.bPush("electron_index", int(el.index))
            self.ntup.bPush("electron_track", getMomentumLV(el.track))
            self.ntup.bPush("electron_cluster", el.cluster)

        for lep in event.leptons:
            self.ntup.bPush("lepton", lep.lv)
            self.ntup.bPush("lepton_charge", int(lep.charge))
        for jet in event.selected_jets:
            self.ntup.bPush("jet", getMomentumLV(jet))
        self.ntup.bSet("ll", event.l1.lv + event.l2.lv)
        met_x, met_y = event.met
        self.ntup.bSet("met_x", met_x)
        self.ntup.bSet("met_y", met_y)
        self.ntup.bSet("met_phi", event.met_phi)
        self.ntup.bSet("met_proj", event.met_proj)
        self.ntup.bSet("event_weight", self.event_weight)
        self.ntup.bSet("event_number", self.event_number)
        self.ntup.bSet("run_number", self.run_number)
        self.ntup.bSet("electron_trigger", 1 if event.passed_triggers("ee") else 0)
        self.ntup.bSet("muon_trigger", 1 if event.passed_triggers("mumu") else 0)
        self.ntup.bSet("vx_good_number", len(event.good_vertices))
        self.ntup.bSet("vx0_good", len(event.vertices) > 0 and self.good_vertex(event.vertices[0]))
        self.ntup.fill()

    def common_init(self):
        self.initialize_trigger()
        self.initialize_stream_overlap()
        self.initialize_jets()
        self.initialize_ntuple()
        
        if self.year == 2010: 
            gROOT.ProcessLine(".L checkOQ.C++")
            from ROOT import egammaOQ
            EAnalysisElectron.egOQ = egammaOQ()
            EAnalysisElectron.egOQ.initialize()

        if self.do_tp_zmm:
            self.initialize_tp_zmm_tree()

        self.tool_ttv = PyAthena.py_tool("Reco::TrackToVertex", iface="Reco::ITrackToVertex")
        self.tool_tit = PyAthena.py_tool("TrackIsolationTool", iface="IIsolationTool")
        #self.tool_tst = PyAthena.py_tool("TrackSummaryTool", iface="ITrackSummaryTool")
        
        if self.reweight == 2010:
            def reweight_vtx_2010(e):
                if self.is_mc:
                    n_vtx = len(e.good_vertices())
                    f = self.corrections.VtxReweighting(n_vtx, e.run_number, e.event_number)
                    self.event_weight *= f
            self.tasks.append(reweight_vtx_2010)
        elif self.reweight == 2011:
            gROOT.ProcessLine(".L TPileupReweighting.C+")
            from ROOT import Root
            self.t_pileup = Root.TPileupReweighting("wwPileup")
            self.t_pileup_initialized = False

            def reweight_vtx_2011(e):
                if self.is_mc:
                    if not self.t_pileup_initialized:
                        self.t_pileup_initialized = True
                        print "INITIALIZING PILEUP"
                        #assert self.t_pileup.initialize("data11_7TeV.periodB.root", "readyMuDist", "mu_mc10a.root", "mu_mc10a") == 0
                        assert self.t_pileup.initialize("ilumicalc_histograms_EF_mu20_MG_178044-182519.root", "avgintperbx", "mu_mc10b.root", "mu_mc10b") == 0
                        #assert self.t_pileup.initialize("ilumicalc_histograms_EF_mu20_MG_178044-183021.root", "avgintperbx", "mu_mc10b.root", "mu_mc10b") == 0
                    f = self.t_pileup.getPileupWeight(e.lumi_block)
                    if f == -1:
                        f = 1.0
                        print e.lumi_block, "MC bin is 0 - probably wrong MC description used??"
                    assert f >= 0, f
                    self.event_weight *= f
            self.tasks.append(reweight_vtx_2011)

class TagProbe(object):

    def initialize_tp_zmm_tree(self, name="TP_ZMM"):
        if not hasattr(self, "tp_tree_tag_names"):
            self.tp_tree_tag_names = {}
        if not hasattr(self, "tp_tree"):
            self.tp_tree = {}
        self.tp_tree_tag_names[name] = ("id", "ms", "msid", "trg")
        self.tp_tree[name] = TreeHelper(name, name, hsvc = self.histogram_manager)
        self.tp_tree[name].intBranch("event_number")
        self.tp_tree[name].intBranch("run_number")
        self.tp_tree[name].floatBranch("event_weight")

        self.tp_tree[name].lvBranch("select")
        self.tp_tree[name].intBranch("select_charge")
        for tn in self.tp_tree_tag_names[name]:
            self.tp_tree[name].lvBranch("tag_%s" % tn)
            self.tp_tree[name].intBranch("tag_%s_charge" % tn)
            self.tp_tree[name].lvVBranch("probes_%s" % tn)
            self.tp_tree[name].intVBranch("probes_%s_charge" % tn)

    def fill_tp_tree(self, name, select, tag, probes):
        self.tp_tree[name].execute()
        self.tp_tree[name].bSet("event_weight", self.event_weight)
        self.tp_tree[name].bSet("event_number", self.event_number)
        self.tp_tree[name].bSet("run_number", self.event_number)
        if select:
            v, c = select
            self.tp_tree[name].bSet("select", v)
            self.tp_tree[name].bSet("select_charge", int(c))
        for tn in self.tp_tree_tag_names[name]:
            if tag[tn]:
                v, c = tag[tn]
                self.tp_tree[name].bSet("tag_%s" % tn, v)
                self.tp_tree[name].bSet("tag_%s_charge" % tn, int(c))
                for probe in probes[tn]:
                    v, c = probe
                    self.tp_tree[name].bPush("probes_%s" % tn, v)
                    self.tp_tree[name].bPush("probes_%s_charge" % tn, int(c))
        self.tp_tree[name].fill()

    def good_track(self, track, pt=15*GeV):
        if track.pt() <= pt or abs(track.eta()) >= 2.4:
            print "track failed pt/eta"
            return False
        summary = track.trackSummary()
        if not summary:
            print "track failed summary"
            return False
        def st(type):
            return summary.get(type)
        if not (not st(SummaryType.expectBLayerHit) or st(SummaryType.numberOfBLayerHits) >= 1):
            print "track failed blayer"
            return False
        if not (st(SummaryType.numberOfPixelHits) + st(SummaryType.numberOfPixelDeadSensors) > 1):
            print "track failed pixel"
            return False
        if not (st(SummaryType.numberOfSCTHits) + st(SummaryType.numberOfSCTDeadSensors) >= 6):
            print "track failed sct"
            return False
        if not (st(SummaryType.numberOfPixelHoles)  +  st(SummaryType.numberOfSCTHoles) < 2):
            print "track failed holes"
            return False

        def good_trt():
            hits = st(SummaryType.numberOfTRTHits)
            outliers = st(SummaryType.numberOfTRTOutliers)
            n = hits + outliers
            if abs(track.eta()) < 1.9:
                return n > 5 and outliers < 0.9 * n
            else:
                return n <= 5 or outliers < 0.9 * n

        if not good_trt():
            print "track failed trt"
            return False
        # the 0 below is "summedPt" http://alxr.usatlas.bnl.gov/lxr/source/atlas/Reconstruction/RecoTools/IsolationTool/IsolationTool/TrackIsolPtType.h#013
        if not self.tool_tit.trackIsolation(track, 0.2, 0, True, array("I",[0]))/track.pt() < 0.1: 
            print "track failed isolation"
            return False

        vxp = self.vertices[0].recVertex().position()
        pavV0 = self.tool_ttv.perigeeAtVertex(track, vxp)
        vx_id_d0_err = pavV0.parameters()[0]/pavV0.localErrorMatrix().error(0)
        vx_id_z0 = pavV0.parameters()[1]
        if not abs(vx_id_d0_err) < 10:
            print "track failed d0"
            return False
        if not abs(vx_id_z0) < 10.0:
            print "track failed z0"
            return False
        return True


    def execute_tp_zmm_tree(self, cg_results, event):
        id_ms_dR = 0.2
        z_window = 10*GeV
        name = "TP_ZMM"

        cuts = ['GRL', 'PVG3', 'met_cleaning', 'trigger_mumu', '0jets', 'overlap']
        if all(bool(cg_results[cut]) for cut in cuts):
            if len(self.selected_mus) >= 1 and len(self.selected_els) == 0:
                # now we have a candidate for tag and probe. Use the first muon as select muon
                select = self.selected_mus[0]
                slv = select.lv

                # now search for tags in the ID, MS, ID&MS, and do trigger matching
                tag = dict(zip(self.tp_tree_tag_names[name], [None]*len(self.tp_tree_tag_names[name])))
                probes = {}

                # Select good inner detector tracks that make a Z
                id_candidates = [t for t in self.sg["TrackParticleCandidate"] if abs((getMomentumLV(t.hlv()) + slv).M() - MZ) < z_window]
                print "found %i candidate ID tracks without cuts" % len(id_candidates)
                id_candidates = [t for t in id_candidates if self.good_track(t)]
                print "found %i candidate ID tracks with cuts" % len(id_candidates)
                if len(id_candidates) == 1:
                    tag_id, = id_candidates
                    tag["id"], tagv = (getMomentumLV(tag_id), tag_id.charge()), tag_id.hlv()
                    # Select good probes in the Muon Spectrometer
                    probes_id = [tp for tp in self.sg["MuonboyTrackParticles"] if tp.hlv().deltaR(tagv) < id_ms_dR]
                    probes["id"] = [(getMomentumLV(probe), probe.charge()) for probe in probes_id]
                elif len(id_candidates) > 1:
                    print "OOPS - more than one good Z-making id track discovered!!"

                # Select muons that make a Z
                ms_candidates = [tp for tp in self.sg["MuonboyTrackParticles"] if abs((getMomentumLV(tp) + slv).M() - MZ) < z_window]
                if len(ms_candidates) == 1:
                    tag_ms, = ms_candidates
                    tag["ms"], tagv = (getMomentumLV(tag_ms), tag_ms.charge()), tag_ms.hlv()
                    # Select good probes in the MS
                    probes_ms = [t for t in self.sg["TrackParticleCandidate"] if t.hlv().deltaR(tagv) < id_ms_dR]
                    probes_ms = [t for t in probes_ms if self.good_track(t)]
                    probes["ms"] = [(getMomentumLV(probe), probe.charge()) for probe in probes_ms]
                elif len(ms_candidates) > 1:
                    print "OOPS - more than one good Z-making ms track discovered!!"
                    for c in ms_candidates:
                        print "candidate ", c.charge(), c.hlv().eta(), c.hlv().phi(), c.hlv().perp()

                # Select id tracks + ms tracks which both make a good Z
                idms_candidates = [id for id in id_candidates if any(id.hlv().deltaR(ms.hlv()) < id_ms_dR for ms in ms_candidates)]
                print "found %i candidate ID+MS tracks" % len(idms_candidates)
                if len(idms_candidates) == 1:
                    tag_idms, = idms_candidates
                    tag["idms"], tagv = (getMomentumLV(tag_idms), tag_idms.charge()), tag_idms.hlv()
                    # Select good combined muons
                    probes_idms = [m for m in self.selected_mus if m.mu.hlv().deltaR(tagv) < id_ms_dR]
                    probes["idms"] = [(probe.lv, probe.charge) for probe in probes_idms]
                elif len(idms_candidates) > 1:
                    print "OOPS - more than one good Z-making id+ms track discovered!!"

                # Fill the tree with all tags and probes
                self.fill_tp_tree("TP_ZMM", (slv, select.charge), tag, probes)




class WZObjects(JEAnalysis, TagProbe):
    def __init__(self, name, year, *args, **kwargs):
        self.year = year
        super(WZObjects, self).__init__(name, *args, **kwargs)


    def init(self):
        self.common_init()
        name = "ww" if self.systematic is None else "ww_"+self.systematic
        ww = self.ww = CutGroup("ww", self.h)

        self.init_cg(ww)

        self.setSkimFilter(lambda e : bool(self.ww.results["l2+"]))

        ww.initialize()

        cuts = ['GRL', 'PVG3', 'met_cleaning', 'trigger_emu', 'l2+', 'overlap']#, "no_jet_in_feb", "no_el_in_feb"]
        def print_cuts(e):
            print "Cut Results:"
            for cut in cuts:
                if bool(e.ww.results[cut]):
                    print "passed ", cut
                else:
                    print "failed ", cut
        #self.tasks.append(print_cuts)
        def ntuple(e):
            if all(bool(e.ww.results[cut]) == True for cut in cuts):
                self.fill_ntuple(e)
        self.tasks.append(ntuple)

        if self.do_tp_zmm:
            def tp(e):
                self.execute_tp_zmm_tree(ww.results, e)
            self.tasks.append(tp)

    def muon_cuts(self, cg, pt, tight_muons, msid_cuts):
        #cg(Cut("D3PD", lambda mu : self.passed_d3pd))

        if not tight_muons:
            if self.muon_algo == "Staco":
                cmb = lambda mu : mu.staco_combined
            else:
                cmb = lambda mu : mu.muid_combined
            cg(Cut("author", cmb))
        else:
            cg(Cut("tight", lambda mu : mu.tight))

        cg(Cut("pt", lambda mu : mu.pt > pt))

        if msid_cuts:
            if self.muon_algo == "Staco" and tight_muons:
                #cg(Cut("chi2", lambda mu : mu.mu.matchChi2() < 150))
                cg(Cut("ptms", lambda mu : mu.tight and ((not mu.staco_combined) or mu.ms_ex.Pt() > 10*GeV)))
                def msid_t(mu):
                    if not mu.staco_combined:
                        return mu.tight
                    ptms = mu.ms_ex.Pt()
                    ptid = mu.id.Pt()
                    if ptid > 0:
                        return abs((ptms - ptid)/ptid) < 0.5
                    else:
                        return False
                cg(Cut("msid", msid_t))
            elif self.muon_algo == "Staco":
                #cg(Cut("chi2", lambda mu : mu.mu.matchChi2() < 150))
                cg(Cut("ptms", lambda mu : mu.ms_ex.Pt() > 10*GeV, cmb))
                def msid_t(mu):
                    ptms = mu.ms_ex.Pt()
                    ptid = mu.id.Pt()
                    if ptid > 0:
                        return abs((ptms - ptid)/ptid) < 0.5
                    else:
                        return False
                cg(Cut("msid", msid_t, cmb))

        def st(mu, type_id):
            idtp = mu.mu.inDetTrackParticle()
            if not idtp:
                raise CutNotApplicable
            ts = idtp.trackSummary()
            if not ts:
                raise CutNotApplicable
            return ts.get(type_id)

        cg(Cut("blayer", lambda mu : not st(mu, SummaryType.expectBLayerHit) or st(mu, SummaryType.numberOfBLayerHits) >= 1))
        cg(Cut("pixel", lambda mu : st(mu, SummaryType.numberOfPixelHits) + st(mu, SummaryType.numberOfPixelDeadSensors) > 1))
        cg(Cut("sct", lambda mu : st(mu, SummaryType.numberOfSCTHits) + st(mu, SummaryType.numberOfSCTDeadSensors) >= 6))
        cg(Cut("si_holes", lambda mu :  st(mu, SummaryType.numberOfPixelHoles)  +  st(mu, SummaryType.numberOfSCTHoles) < 2))
        cg(Cut("trt", lambda mu : mu.good_trt))

        cg(Cut("eta", lambda mu : abs(mu.eta) < 2.4))

        if self.loose_leptons:
            return cg

        cg(Cut("vx_z0", lambda mu : abs(self.vx_id_z0(mu)) < 10.0))
        cg(Cut("vx_d0", lambda mu : abs(self.vx_id_d0_err(mu)) < 10)) 

        cg(Cut("ptcone20", lambda mu : mu.mu.parameter(EAnalysisMuon.MuonParameters.ptcone20)/mu.pt < 0.1))

        return cg

    def electron_cuts(self, cg, pt, relative_isolation=False, higgs_pt=False):
        # container selection electrons - all standalone
        # Electron Filters:
        #cg(Cut("D3PD", lambda mu : self.passed_d3pd))
        cg(Cut("author", lambda el : el.author(1) or el.author(3)))
        def author(el):
            return el.author(1) or el.author(3)

        if self.year == 2011:
            cg(Cut("oq", lambda el : el.el.isgoodoq(EAnalysisElectron.egammaPID.BADCLUSELECTRON) == 0, dep=author))
        elif self.year == 2010:
            cg(Cut("oq", lambda el : el.pass_otx(self.run_number, self.is_mc), dep=author))

        cg(Cut("eta", lambda el: abs(el.cluster_eta) < 2.47 and not el.in_crack_region, dep=author))

        if self.year == 2010:
            cg(Cut("tight", lambda el : el.el.isElectron(EAnalysisElectron.egammaPID.ElectronTight_WithTrackMatch), dep=author))
        else:
            cg(Cut("tight", lambda el : el.el.isElectron(EAnalysisElectron.egammaPID.ElectronTight), dep=author))

        if higgs_pt:
            cg(Cut("pt", lambda el : el.higgs_pt > pt, dep=author))
        else:
            cg(Cut("pt", lambda el : el.lv.Et() > pt, dep=author))

        if self.loose_leptons:
            return cg

        cg(Cut("vx_z0", lambda el : abs(self.vx_id_z0(el)) < 10.0, 
                            dep=lambda el : author(el) and self.vertices))
        cg(Cut("vx_d0", lambda el : abs(self.vx_id_d0_err(el)) < 10, 
                            dep=lambda el : author(el) and self.vertices))

        if relative_isolation:
            cg(Cut("etcone20", lambda el : el.el.detailValue(EAnalysisElectron.egammaParameters.etcone20)/el.lv.Et() < 0.15, dep=author))
            cg(Cut("ptcone20", lambda el : el.el.detailValue(EAnalysisElectron.egammaParameters.ptcone20)/el.lv.Et() < 0.1 , dep=author))
        else:
            cg(Cut("etcone30", lambda el : el.el.detailValue(EAnalysisElectron.egammaParameters.etcone30) < 6*GeV))
        return cg

    def jet_cuts(self, cg, pt, eta):
    # container selection jets - all standalone
        # Jet Filters
        #cg(Cut("D3PD", lambda mu : self.passed_d3pd))
        cg(Cut("pt", lambda jet : jet.pt() > pt))
        cg(Cut("eta", lambda jet : abs(jet.hlv(JETEMSCALE).eta()) < eta))
        cg(Cut("bad", lambda jet : self.is_mc or (not self.jet_bad(jet))))
        cg(Cut("ugly", lambda jet : self.is_mc or (not self.jet_ugly(jet))))

    def common_cuts(self, cg, electron_name, jet_name, use_higgs_lv=False):

        cg(SetContainer("preOR_els", lambda e : cg.selected(e, electron_name)))
        cg(SetContainer("preOR_jets", lambda e : cg.selected(e, jet_name)))
        cg(SetContainer("midOR_els", selfoverlap_func("preOR_els", lambda el : el.cluster, 0.1)))
        cg(SetContainer("selected_els", or_func("midOR_els", lambda el : el.cluster, lambda e : (mu.lv for mu in e.selected_mus), 0.1)))
        if use_higgs_lv:
            cg(SetContainer("selected_jets", or_func("preOR_jets", lambda jet : getMomentumLV(jet.hlv(JETEMSCALE)), lambda e : (el.higgs_lv for el in e.selected_els), 0.3)))
        else:
            cg(SetContainer("selected_jets", or_func("preOR_jets", lambda jet : getMomentumLV(jet.hlv(JETEMSCALE)), lambda e : (el.cluster for el in e.selected_els), 0.3)))

        cg(Cut("e_m_overlap", lambda e : len(e.midOR_els) != len(e.selected_els)))
        cg(Cut("no_jet_in_feb", lambda e : not any(FEB_pass_jet(j.hlv(JETEMSCALE)) for j in e.selected_jets)))
        cg(Cut("no_el_in_feb", lambda e : not any(e.in_feb for e in e.selected_els)))
        def feb_cut(e):
            if any(FEB_pass_jet(j.hlv(JETEMSCALE)) for j in e.selected_jets) or any(e.in_feb for e in e.selected_els):
                e.event_weight *= 163.502/205.236
                return False
            return True
        cg(Cut("feb", feb_cut))
        cg(Cut("l1+", lambda e : len(e.leptons) >= 1 ))
        cg(Cut("l2+", lambda e : len(e.leptons) >= 2 ))
        cg(Cut("l=2", lambda e : len(e.leptons) == 2 ))
        cg(Cut("1e", lambda e : len(e.selected_els) >= 1 ))
        cg(Cut("1m", lambda e : len(e.selected_mus) >= 1 ))
        cg(Cut("ee", lambda e : len(e.selected_els) == 2 and len(e.selected_mus) == 0))
        cg(Cut("em", lambda e : len(e.selected_els) == 1 and len(e.selected_mus) == 1))
        cg(Cut("mm", lambda e : len(e.selected_els) == 0 and len(e.selected_mus) == 2))

        #independent cuts
        cg(Cut("D3PD", lambda e : e.passed_d3pd))
        cg(Cut("GRL", lambda e : e.passed_grl))
        cg(Cut("overlap", lambda e : not e.is_overlap))
        for c in ("mumu", "ee", "emu"):
            cg(Cut("trigger_%s"%c, lambda e, c=c : e.passed_triggers(c)))
        cg(Cut("PV0", lambda e : len(e.vertices) > 0))
        cg(Cut("PVG3", lambda e : len(e.vertices) > 0 and len(e.vertices[0].vxTrackAtVertex()) >= 3))
        cg(Cut("PVG5", lambda e : len(e.vertices) > 0 and len(e.vertices[0].vxTrackAtVertex()) >= 5))
        cg(Cut("met_cleaning", lambda e : not any(e.is_cleaning_jet(j) for j in e.jets)))
        cg(Cut("neg_sumet", lambda e : self.sg["MET_RefFinal"].sumet() >= 0))

    def more_common_cuts(self, cg):
        ll = lambda e : len(e.leptons) >= 2
        cg(Cut("l1_20GeV", lambda e : (len(e.leptons) >= 1 and max([l.pt for l in e.leptons]) > 20*GeV)))
        cg(Cut("l1_25GeV", lambda e : (len(e.leptons) >= 1 and max([l.pt for l in e.leptons]) > 25*GeV)))

        cg(Cut("OS", lambda e : e.l1.charge * e.l2.charge < 0, dep=ll))
        cg(Cut("SS", lambda e : e.l1.charge * e.l2.charge > 0, dep=ll))

        cg(Cut("llpT_gt30", lambda e : e.ll.Pt() > 30*GeV, dep=ll))
        cg(Cut("llm_gt15", lambda e : e.ll.M() > 15*GeV, dep=ll))
        cg(Cut("llm_mZ10", lambda e : abs(e.ll.M() - MZ) > 10*GeV, dep=ll))
        cg(Cut("llm_mZ15", lambda e : abs(e.ll.M() - MZ) > 15*GeV, dep=ll))
        cg(Cut("llm_lt50", lambda e : e.ll.M() < 50*GeV, dep=ll))
        cg(Cut("llm_lt65", lambda e : e.ll.M() < 65*GeV, dep=ll))

        cg(Cut("ll_dphi_lt13", lambda e : dphi(e.l1.phi, e.l2.phi) < 1.3, dep=ll))
        cg(Cut("ll_dphi_lt18", lambda e : dphi(e.l1.phi, e.l2.phi) < 1.8, dep=ll))

        cg(Cut("D_z0_id", lambda e : abs(e.get_z0_id(e.l1) - e.get_z0_id(e.l2)) < 0.8, dep=ll))
        cg(Cut("D_z0_id", lambda e : abs(e.get_z0_id(e.l1) - e.get_z0_id(e.l2)) < 0.8, dep=ll))

        cg(Cut("METproj_gt20", lambda e : e.met_proj > 20*GeV, dep=ll))
        cg(Cut("METproj_gt25", lambda e : e.met_proj > 25*GeV, dep=ll))
        cg(Cut("METproj_gt40", lambda e : e.met_proj > 40*GeV, dep=ll))
        cg(Cut("MET_gt30", lambda e : e.met_et > 30*GeV))
        cg(Cut("MET_gt40", lambda e : e.met_et > 40*GeV))

        cg(Cut("leq2jets", lambda e : len(e.selected_jets) <= 2))
        cg(Cut("leq1jet", lambda e : len(e.selected_jets) <= 1))
        cg(Cut("0jets", lambda e : len(e.selected_jets) == 0))

        cg(Cut("c_sumeta", lambda e : abs(e.l1.eta + e.l2.eta) < 0.0033, dep=ll))
        cg(Cut("c_dphi", lambda e : dphi(e.l1.phi, e.l2.phi) > 3.14, dep=ll))

    def muon_quality_histos(self, cg, reqs, prefix="", cumulative=False):
        reqs = list(reqs)
        pf = prefix
        cg(Histo(name="%spt"%pf, b=[(10000,0,1000)], value = lambda mu : (mu.pt/GeV), req=reqs))
        if cumulative: reqs.append("pt")
        cg(Histo(name="%seta"%pf, b=[(1000,-5,5)], value = lambda mu : (mu.eta), req=reqs))
        if cumulative: reqs.append("eta")
        cg(Histo(name="%spixel"%pf, b=[(100,0,100)], value = lambda mu : mu.mu.numberOfPixelHits(), req=reqs))
        if cumulative: reqs.append("pixel")
        cg(Histo(name="%ssct"%pf, b=[(100,0,100)], value = lambda mu : mu.mu.numberOfSCTHits(), req=reqs))
        if cumulative: reqs.append("sct")
        cg(Histo(name="%strt_hits"%pf, b=[(100,0,100)], value = lambda mu : mu.mu.numberOfTRTHits(), req=reqs))
        cg(Histo(name="%strt_outliers"%pf, b=[(100,0,100)], value = lambda mu : mu.mu.numberOfTRTOutliers(), req=reqs))
        if cumulative: reqs.append("trt")
        if self.muon_algo == "Staco":
            cg(Histo(name="%spt_ms"%pf, b=[(1000,0,1000)], value = lambda mu : mu.ms.Pt()/GeV, req=reqs))
            #if cumulative: reqs.append("ptms")
            cg(Histo(name="%spt_ms_ex"%pf, b=[(1000,0,1000)], value = lambda mu : mu.ms_ex.Pt()/GeV, req=reqs))
            cg(Histo(name="%spt_ms_id"%pf, b=[(1000,0,1000)], value = lambda mu : mu.id.Pt()/GeV, req=reqs))
            cg(Histo(name="%spt_ms_ex_minus_id"%pf, b=[(1000,-100,100)], value = lambda mu : mu.ms_ex.Pt()/GeV - mu.id.Pt()/GeV, req=reqs))
            cg(Histo(name="%spt_msid_qual_gt_neg04"%pf, b=[(1000,-100,100)], value = lambda mu : (mu.ms_ex.Pt()/(mu.id.Pt()+0.001) - 1), req=reqs))
            #if cumulative: reqs.append("msid")
        cg(Histo(name="%sd0_err"%pf, b=[(2000,-100,100)], value = lambda mu : self.vx_id_d0_err(mu), req=reqs))
        if cumulative and not self.loose_leptons: reqs.append("vx_d0")
        cg(Histo(name="%sz0"%pf, b=[(2000,-100,100)], value = lambda mu : self.vx_id_z0(mu), req=reqs))
        if cumulative and not self.loose_leptons: reqs.append("vx_z0")
        #cg(Histo(name="%setcone30"%pf, b=[(1000,0,100)], value = lambda mu : mu.mu.parameter(EAnalysisMuon.MuonParameters.etcone30)/GeV, req=reqs))
        #cg(Histo(name="%setcone30_vs_pt"%pf, b=[(100,0,50),(500,0,500)], value = lambda mu : (mu.mu.parameter(EAnalysisMuon.MuonParameters.etcone30)/GeV, mu.pt/GeV), req=reqs))
        #if cumulative and not self.loose_leptons: reqs.append("etcone30")
        cg(Histo(name="%sptcone30"%pf, b=[(1000,0,100)], value = lambda mu : mu.mu.parameter(EAnalysisMuon.MuonParameters.ptcone30)/GeV, req=reqs))
        cg(Histo(name="%sptcone30_vs_pt"%pf, b=[(100,0,50),(500,0,500)], value = lambda mu : (mu.mu.parameter(EAnalysisMuon.MuonParameters.ptcone30)/GeV, mu.pt/GeV), req=reqs))
        #if cumulative: reqs.append("ptcone30") # if any more histograms are added

    def electron_quality_histos(self, cg, reqs, prefix="", cumulative=False):
        reqs = list(reqs)
        pf = prefix
        cg(Histo(name="%scluster_eta"%pf, b=[(1000,-5,5)], value = lambda el : (el.cluster_eta), req=reqs))
        if cumulative: reqs.append("eta")
        cg(Histo(name="%scluster_pt"%pf, b=[(10000,0,1000)], value = lambda el : (el.cluster_pt/GeV), req=reqs))
        if cumulative: reqs.append("pt")
        cg(Histo(name="%sz0"%pf, b=[(2000,-100,100)], value = lambda el : self.vx_id_z0(el), req=reqs))
        if cumulative and not self.loose_leptons: reqs.append("vx_z0")
        cg(Histo(name="%setcone30"%pf, b=[(1000,0,100)], value = lambda el : el.el.detailValue(EAnalysisElectron.egammaParameters.etcone30)/GeV, req=reqs))
        cg(Histo(name="%setcone30_vs_pt"%pf, b=[(100,0,50),(500,0,500)], value = lambda el : (el.el.detailValue(EAnalysisElectron.egammaParameters.etcone30)/GeV, el.cluster_pt/GeV), req=reqs))
        def dR_mu(el):
            return min([100] + [el.DeltaR(mu) for mu in self.selected_mus])
        if cumulative:
            cg(Histo("e_m_dR_trk_noniso", b=[(10000,0,10)], value = lambda el : dR_mu(getMomentumLV(el.track)), req=reqs))
            cg(Histo("e_m_dR_cls_noniso", b=[(10000,0,10)], value = lambda el : dR_mu(el.cluster), req=reqs))
        #if cumulative and not self.loose_leptons: reqs.append("etcone30")
        if cumulative:
            cg(Histo("e_m_dR_trk_iso", b=[(10000,0,10)], value = lambda el : dR_mu(getMomentumLV(el.track)), req=reqs))
            cg(Histo("e_m_dR_cls_iso", b=[(10000,0,10)], value = lambda el : dR_mu(el.cluster), req=reqs))
        cg(Histo(name="%sptcone30"%pf, b=[(1000,0,100)], value = lambda el : el.el.detailValue(EAnalysisElectron.egammaParameters.ptcone30)/GeV, req=reqs))
        cg(Histo(name="%sptcone30_vs_pt"%pf, b=[(100,0,50),(500,0,500)], value = lambda el : (el.el.detailValue(EAnalysisElectron.egammaParameters.ptcone30)/GeV, el.cluster_pt/GeV), req=reqs))


    def jet_quality_histos(self, cg, reqs, prefix=""):
        reqs = list(reqs)
        pf = prefix
        cg(Histo(name="%spt"%pf, b=[(10000,0,1000)], value = lambda jet : jet.pt()/GeV, req=reqs))
        reqs.append("pt")
        cg(Histo(name="%seta"%pf, b=[(1000,-5,5)], value = lambda jet : jet.hlv(JETEMSCALE).eta(), req=reqs))
        reqs.append("eta")
        cg(Histo(name="%sjvf"%pf, b=[(100,0,1)], value = lambda jet : 1.0 if abs(jet.eta()) >= 2.1 else jet.getMoment("PRIM_VTX_F"), req=reqs))

    def dilepton_histograms(self, cg, reqs, prefix):
        pf = prefix
        cg(Histo(name="%sl1_pt"%pf, b=[(10000,0,1000)], value = lambda e : (e.l1.pt/GeV), req=reqs))
        cg(Histo(name="%st1_eta"%pf, b=[(1000,-5,5)], value = lambda e : (e.l1.eta), req=reqs))
        cg(Histo(name="%sl1_phi"%pf, b=[(1000,-pi,pi)], value = lambda e : (e.l1.phi), req=reqs))
        cg(Histo(name="%sl2_pt"%pf, b=[(10000,0,1000)], value = lambda e : (e.l2.pt/GeV), req=reqs))
        cg(Histo(name="%st2_eta"%pf, b=[(1000,-5,5)], value = lambda e : (e.l2.eta), req=reqs))
        cg(Histo(name="%sl2_phi"%pf, b=[(1000,-pi,pi)], value = lambda e : (e.l2.phi), req=reqs))
        cg(Histo(name="%sll_pt"%pf, b=[(500,0,500)], value = lambda e : (e.ll.Pt()/GeV,), req=reqs))
        cg(Histo(name="%sll_eta"%pf, b=[(500,-5,5)], value = lambda e : (e.ll.Eta(),), req=reqs))
        cg(Histo(name="%sll_m"%pf, b=[(500,0,500)], value = lambda e : (e.ll.M()/GeV,), req=reqs))
        cg(Histo(name="%sll_dphi"%pf, b=[(10000, -pi, 2*pi)], value = lambda e : dphi(e.l1.phi, e.l2.phi), req=reqs))
        cg(Histo(name="%sll_sumeta"%pf, b=[(2000,-10,10),], value = lambda e : (e.l1.eta + e.l2.eta), req=reqs))
        cg(Histo(name="%sll_sumeta_dphi"%pf, b=[(200,-0.01,0.01),(3141, 0, pi)], value = lambda e : (e.l1.eta + e.l2.eta, dphi(e.l1.phi, e.l2.phi)), req=reqs))

    def kinematic_histos(self, cg, reqs, prefix=""):
        pf = prefix
        cg(Histo)
        cg(Histo(name="%spt"%pf, b=[(10000,0,1000)], value = lambda obj : (obj.pt/GeV), req=reqs))
        cg(Histo(name="%seta"%pf, b=[(1000,-5,5)], value = lambda obj : (obj.eta), req=reqs))
        cg(Histo(name="%sphi"%pf, b=[(1000,-pi,pi)], value = lambda obj : (obj.phi), req=reqs))
        cg(Histo(name="%seta_phi"%pf, b=[(100,-5,5),(100,-pi,pi)], value = lambda obj : (obj.eta, obj.phi), req=reqs))


class WWAnalysis(WZObjects):
    def init_cg(self, ww):
        wwn = ww.name
        self.muon_cuts(ww.make_subgroup("muons", "mus"), pt=20*GeV, tight_muons=False, msid_cuts=False)
        ww(SetContainer("selected_mus", lambda e : ww.selected(e, wwn+"/muons")))
        self.electron_cuts(ww.make_subgroup("electrons", "els"), pt=20*GeV)
        self.jet_cuts(ww.make_subgroup("jets", "jets"), pt=20*GeV, eta=4.4)
        self.common_cuts(ww, wwn+"/electrons", wwn+"/jets")
        self.more_common_cuts(ww)

        ww(Histo(name="ee_or", b=[(50,0,50)], value = lambda e : len(e.midOR_els)))
        ww(Histo(name="em_or", b=[(50,0,50)], value = lambda e : len(e.selected_els)))
        ww(Histo(name="je_or", b=[(50,0,50)], value = lambda e : len(e.selected_jets)))

        ll = lambda e : len(e.leptons) >= 2
        if self.make_histograms:
            # object-level histograms
            qhist = "author"
            histo_reqs = ["GRL", "overlap", "PVG3", "met_cleaning", "trigger_mumu"]
            self.muon_quality_histos(ww.sg[wwn+"/muons"], histo_reqs + [qhist], "ini_")
            self.muon_quality_histos(ww.sg[wwn+"/muons"], histo_reqs + [qhist], "cum_", cumulative=True)
            self.electron_quality_histos(ww.sg[wwn+"/electrons"], histo_reqs + [qhist], "ini_")
            self.electron_quality_histos(ww.sg[wwn+"/electrons"], histo_reqs + [qhist], "cum_", cumulative=True)
            self.kinematic_histos(ww.sg[wwn+"/muons"], histo_reqs + [qhist], "author_")
            self.kinematic_histos(ww.sg[wwn+"/muons"], histo_reqs + ww.sg[wwn+"/muons"].cut_names, "good_")
            self.kinematic_histos(ww.sg[wwn+"/electrons"], histo_reqs + ["author"], "author_")
            self.kinematic_histos(ww.sg[wwn+"/electrons"], histo_reqs + ww.sg[wwn+"/electrons"].cut_names, "good_")
            self.jet_quality_histos(ww.sg[wwn+"/jets"], ["GRL", "overlap", "trigger_mumu"], "cum_")

            # analysis-level histograms
            ll_reqs = histo_reqs + ["l=2"]
            self.dilepton_histograms(ww, ll_reqs, "presel_")
            self.dilepton_histograms(ww, ll_reqs + ["OS"], "os_")
            self.dilepton_histograms(ww, ll_reqs + ["SS"], "ss_")
            self.dilepton_histograms(ww, ll_reqs + ["OS","0jets"], "os_0jet_")
            self.dilepton_histograms(ww, ll_reqs + ["SS","0jets"], "ss_0jet_")

            ww(Histo(name="MET", b=[(300,0,300)], value = lambda e : (e.met_et/GeV,), cross=("l=2",)))
            ww(Histo(name="MET_phi", b=[(100,-pi,pi)], value = lambda e : (e.met_phi,) ))
            ww(Histo(name="MET_pt_phi", b=[(50,0,200),(10,-pi,pi)], value = lambda e : (e.met_et/GeV, e.met_phi) ))
            ww(Histo(name="n_jets", b=[(20,0,20)], value = lambda e : len(e.selected_jets)))


            ww(Histo(name="mu_d0", b=[(2000,-1000,1000)], value = lambda e : e.selected_mus[0].inner_track.measuredPerigee().parameters()[0], req=("mm",), cross=("trigger_mumu",) ))
            ww(Histo(name="mu_z0", b=[(2000,-1000,1000)], value = lambda e : e.selected_mus[0].inner_track.measuredPerigee().parameters()[1], req=("mm",), cross=("trigger_mumu",) ))
            ww(Histo(name="sum_eta_xtrig", b=[(200000,-1,1)], value = lambda e : e.l1.eta + e.l2.eta, req=("mm", "GRL"), cross=("trigger_mumu",) ))
            ww(Histo(name="sum_eta_rtrig_xmetclean", b=[(200000,-1,1)], value = lambda e : e.l1.eta + e.l2.eta, 
                                                     req=("mm", "GRL", "OS", "trigger_mumu"), cross=("met_cleaning",) ))
            ww(Histo(name="sum_eta_rtrig_xmetclean", b=[(200000,-1,1)], value = lambda e : e.l1.eta + e.l2.eta, 
                                                     req=("mm", "GRL", "OS", "trigger_mumu"), cross=("met_cleaning",) ))
            ww(Histo(name="sum_eta_rall_xllpt", b=[(200000,-1,1)], value = lambda e : e.l1.eta + e.l2.eta, 
                                                     req=("mm", "GRL", "OS", "trigger_mumu", "met_cleaning", "D_z0_id", "llm_gt15", "llm_mZ10", "0jets"), cross=("llpT_gt30",) ))
            ww(Histo(name="sumeta_phi1_phi2", b=[(2000,-0.01,0.01),(100,-pi, pi),(100,-pi,pi)], 
                     value = lambda e : (e.l1.eta + e.l2.eta, e.l1.phi, e.l2.phi), 
                     req=("mm", "GRL") ))
            ww(Histo(name="sumeta_dphi", b=[(200,-0.01,0.01),(3141, 0, pi)],
                     value = lambda e : (e.l1.eta + e.l2.eta, dphi(e.l1.phi, e.l2.phi)), 
                     req=("mm", "GRL") ))

            ww(Histo(name="mm_dphi", b=[(10000, 0, pi)],
                     value = lambda e : dphi(e.l1.phi, e.l2.phi), 
                     req=("mm",) ))


class HiggsAnalysis(WZObjects):
    def init_cg(self, ww):
        wwn = ww.name
        self.muon_cuts(ww.make_subgroup("muons", "mus"), pt=15*GeV, tight_muons=True, msid_cuts=True)
        ww(SetContainer("selected_mus", lambda e : ww.selected(e, wwn+"/muons")))
        self.electron_cuts(ww.make_subgroup("electrons", "els"), pt=15*GeV, relative_isolation=True, higgs_pt=True)
        self.jet_cuts(ww.make_subgroup("jets", "jets"), pt=20*GeV, eta=3)
        self.common_cuts(ww, wwn+"/electrons", wwn+"/jets")
        self.more_common_cuts(ww)

        ll = lambda e : len(e.leptons) >= 2
        # object-level histograms
        histo_reqs = ["GRL", "overlap", "PVG3", "met_cleaning", "trigger_mumu"]
        self.muon_quality_histos(ww.sg[wwn+"/muons"], histo_reqs + ["tight"], "ini_")
        self.muon_quality_histos(ww.sg[wwn+"/muons"], histo_reqs + ["tight"], "cum_", cumulative=True)
        self.electron_quality_histos(ww.sg[wwn+"/electrons"], histo_reqs + ["tight"], "ini_")
        self.electron_quality_histos(ww.sg[wwn+"/electrons"], histo_reqs + ["tight"], "cum_", cumulative=True)
        self.kinematic_histos(ww.sg[wwn+"/muons"], histo_reqs + ["tight"], "tight_")
        self.kinematic_histos(ww.sg[wwn+"/muons"], histo_reqs + ww.sg[wwn+"/muons"].cut_names, "good_")
        self.kinematic_histos(ww.sg[wwn+"/electrons"], histo_reqs + ["author"], "author_")
        self.kinematic_histos(ww.sg[wwn+"/electrons"], histo_reqs + ww.sg[wwn+"/electrons"].cut_names, "good_")
        self.jet_quality_histos(ww.sg[wwn+"/jets"], ["GRL", "overlap", "trigger_mumu"], "cum_")

        # analysis-level histograms
        ll_reqs = histo_reqs + ["l=2"]
        self.dilepton_histograms(ww, ll_reqs, "presel_")
        self.dilepton_histograms(ww, ll_reqs + ["OS"], "os_")
        self.dilepton_histograms(ww, ll_reqs + ["SS"], "ss_")
        self.dilepton_histograms(ww, ll_reqs + ["OS","0jets"], "os_0jet_")
        self.dilepton_histograms(ww, ll_reqs + ["SS","0jets"], "ss_0jet_")

        ww(Histo(name="MET", b=[(300,0,300)], value = lambda e : (e.met_et/GeV,), cross=("l=2",)))
        ww(Histo(name="MET_phi", b=[(100,-pi,pi)], value = lambda e : (e.met_phi,) ))
        ww(Histo(name="MET_pt_phi", b=[(50,0,200),(10,-pi,pi)], value = lambda e : (e.met_et/GeV, e.met_phi) ))
        ww(Histo(name="n_jets", b=[(20,0,20)], value = lambda e : len(e.selected_jets)))

accept_algs = []

if "systematics" in options:
    systematics = options["systematics"]
else:
    systematics = []

def get_std_analysis(typ, year, name=None, hm=None):
    print typ.upper()
    assert typ.upper() in ("HWW", "WWXS")
    if name is None:
        name = "MINTY_%s_STACO" % typ.upper()
    if typ.upper() == "HWW":
        a = HiggsAnalysis(name, year, options, hm)
        a.mu_trigger = "higgs"
        a.use_dilepton_triggers = True
        a.met_loc_had_topo = True
        a.correct_electron_efficiency = "Tight"
    else:
        a = WWAnalysis(name, year, options, hm)
        a.muon_algo = "Staco"
        a.mu_trigger = "ww"
        a.use_dilepton_triggers = False
        a.met_loc_had_topo = False # TODO: revert back after r19 comparison
    a.muon_algo = "Staco"
    a.loose_leptons = False
    a.profile = False
    a.make_histograms = True
    a.do_tp_zmm = True
    a.systematic = None
    accept_algs.append(name)
    return a

# switch here
ana = None
for typ in options["analysis"]:
    ana = get_std_analysis(typ, options["year"], "MINTY_%s_STACO" % typ.upper())
    ana.reweight = 2011
    ana.smear_muons = True
    ana.correct_electrons = True
    ana.correct_electron_efficiency = None

    topSequence += ana
    for systematic in systematics:
        name = "MINTY_%s_STACO_%s" % (typ.upper(), systematic.upper())
        a = get_std_analysis(typ, options["year"], name, ana.histogram_manager)
        a.reweight = ana.reweight
        a.smear_muons = ana.smear_muons
        a.correct_electrons = ana.correct_electrons
        a.systematic = systematic
        topSequence += a

if "skim" in options:
    setup_pool_skim("MintySkim.AOD.pool.root", accept_algs, skim_type)

