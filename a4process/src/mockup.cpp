#include <list>
#include <cmath>

#include "a4/alorentzvector.h"

#include "a4/main.h"
#include "a4/grl.h"
#include "a4/h1.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <utility>

#include "analysis.h"
#include "MuonTriggerSF.h"

#include "PileupReweighting/TPileupReweighting.h"
#include "MuonEfficiencyCorrections/StacoCBScaleEffFactors.h"
#include "MuonEfficiencyCorrections/MuidCBScaleEffFactors.h" 
#include "MuonEfficiencyCorrections/StacoTightScaleEffFactors.h"
#include "MuonEfficiencyCorrections/MuidTightScaleEffFactors.h" 
#include "SmearingClass.h"
#include "EnergyRescaler.h"
#include "checkOQ.h"
#include "egammaAnalysisUtils/egammaSFclass.h"
#include "LArHole.hpp"
#include "RandomDataPeriod.h"
#include "bad_jet.hpp"

using namespace std;

// ATLAS constants
const double GeV = 1000.0;
const double mm = 1.0;
const double m_Z = 91.1876*GeV;

// Utility conversion functions

inline double pow2(double v) { return v*v; }
inline double add_in_sq(double a, double b) { return sqrt(a*a + b*b); };

inline ALorentzVector ALV(LorentzVector p4) {
    return ALorentzVector(p4.px(), p4.py(), p4.pz(), p4.e());
};

inline ALorentzVector ALV(MissingEnergy met) {
    ALorentzVector v(met.x(), met.y(), 0, 0);
    v.E = v.pt();
    return v;
};

inline ALorentzVector ALV(float x, float y) {
    ALorentzVector v(x, y, 0, 0);
    v.E = v.pt();
    return v;
};

inline TLorentzVector TLV(ALorentzVector &alv) {
    return TLorentzVector(alv.px, alv.py, alv.pz, alv.E);
}

double trigger_event_eff(double eff1, double eff2) { 
    return eff1 + eff2 - eff1 * eff2; 
};

double trigger_event_sf(double eff1, double eff2, double sf1, double sf2) { 
    return trigger_event_eff(eff1*sf1, eff2*sf2) / trigger_event_eff(eff1, eff2); 
};

// Analysis helper classes
class AElectron {
    public:
        AElectron(a4::ebke::Electron * j) : pb(j), 
                                  lv(alv(j->p4())), 
                                  cluster(alv(j->p4_cluster())),
                                  track(alv(j->p4_track()))
                                {};
        Electron * pb;
        ALorentzVector lv;
        ALorentzVector cluster;
        ALorentzVector track;
        ALorentzVector met_contribution; // sign convention: this is the "added momentum by changes"
        bool in_crack() const {
            double cl_eta = fabs(cluster.eta());
            return 1.37 < cl_eta && cl_eta < 1.52;
        };
        double good_eta() const {
            // Take cluster or track eta depending on track quality 
            if (pb->track_hits().numberofscthits() + pb->track_hits().numberofpixelhits() >=4 )
                return track.eta();
            else
                return cluster.eta();
        };
        double ET() const {
            return cluster.e()/cosh(good_eta());
        };

};
typedef boost::shared_ptr<AElectron> AElectronPtr;


class AMuon {
    public:
        AMuon(Muon * j) : pb(j), 
                                  lv(alv(j->p4())), 
                                  ms(alv(j->p4_ms())),
                                  track(alv(j->p4_track()))
                                {};
        Muon * pb;
        ALorentzVector lv;
        ALorentzVector ms;
        ALorentzVector track;
        ALorentzVector met_contribution;

};
typedef boost::shared_ptr<AMuon> AMuonPtr;


class ALepton {
  public:
    typedef enum { e=0, m=1, t=2 } Flavor;
    ALepton(AElectron & l) : lv(l.lv), charge(l.pb->charge()), flavor(e) {};
    ALepton(AMuon &l) : lv(l.lv), charge(l.pb->charge()), flavor(m) {};
    ALorentzVector lv;
    int charge;
    Flavor flavor;
    bool operator<(ALepton l2) const { return lv.pt() > l2.lv.pt(); };
};


class AJet {
    public:
        AJet(Jet * j) : pb(j), lv(alv(j->p4())), lv_em(alv(j->p4_em())) {};
        Jet * pb;
        ALorentzVector lv;
        ALorentzVector lv_em;
        bool is_bad() {
            return is_bad_jet(*pb, lv_em.eta(), lv.pt());
        }
};
typedef boost::shared_ptr<AJet> AJetPtr;


class HWWAnalysis : public ProcessorOf<AtlasEvent, AtlasMetaData> {
    public: 
        virtual void process(const AtlasEvent &event);

        void plot_kinematics(const char * tag, ALorentzVector l0, ALorentzVector l1) {
            plot_kinematics_1p(tag "/l0_", l0);
            plot_kinematics_1p(tag "/l1_", l1);
            plot_kinematics_2p(tag "/ll_", l0, l1);
        };

        template<typename ...Args>
        void plot_kinematics_1p(const Args& ...args, const ALorentzVector &v) {
            S<H1>(prefix, "m")(400, 0, 400).fill(v.m()/GeV);
            S<H1>(prefix, "pt")(400, 0, 400).fill(v.pt()/GeV);
            S<H1>(prefix, "phi")(100, -M_PI, M_PI).fill(v.phi());
            S<H1>(prefix, "eta")(100, -5, 5).fill(v.eta());
            S<H2>(prefix, "phi_eta")(20, -M_PI, M_PI, 20, -5, 5).fill(v.phi(), v.eta());
            S<H1>(prefix, "m_1gev").resolution(1*GeV).fill(v.m())
        };

        void plot_kinematics_2p(const char * prefix, const ALorentzVector &v0, const ALorentzVector &v1) {
            ALorentzVector vv = v1 + v2;
            S<H1>(prefix, "mm")(4000, 0, 4000).fill(vv.m()/GeV);
            S<H1>(prefix, "pt")(500, 0, 500).fill(vv.pt()/GeV);
            S<H1>(prefix, "phi")(200, -M_PI, M_PI).fill(vv.phi());
            S<H1>(prefix, "eta")(200, -10, 10).fill(vv.eta());
            S<H1>(prefix, "dphi")(200, -M_PI, M_PI).fill(v1.delta_phi(v2));
            S<H1>(prefix, "deta")(200, -10, 10).fill(v1.eta() - v2.eta());
            S<H1>(prefix, "dR")(200, 0, 10).fill(v1.delta_r(v2));
        }

       // All these pointers don't get cleaned up at the moment. Since they live until
       // the end of the program anyway... They should be Boost shared pointers... 
        Root::TPileupReweighting * prw;
        EnergyRescaler * e_rsc;
        SmearingClass * m_scl;
        egammaOQ * m_checkOQ;
        Analysis::StacoCBScaleEffFactors * m_staco_cb_sf; 
        Analysis::StacoTightScaleEffFactors * m_staco_tight_sf; 
        egammaSFclass * e_sf; 
        MuonTriggerSF * m_tsf;
};

class AEvent : public SerializeTo<a4::atlas::Event> {};

void HWWAnalysis::process_event(const Event &event) {
    AEvent & event = dynamic_cast<AEvent &> event;

    uint32_t event_number = event.event_number();
    uint32_t run_number = event.run_number();
    double event_scale_factor = event.mc_event_weight();
    double muon_scale_factor = 1.0;
    double electron_scale_factor = 1.0;
    bool electron_pointing_at_hole = false;

    if (!data && do_mc_weights) event_scale_factor *= mc_weights[run_number]*lumi_pb;

    // Acquire Analysis objects
    float met_central_x = (event.met_lochadtopo().met_central().x());
    float met_endcap_x = (event.met_lochadtopo().met_endcap().x());
    float met_forward_x = (event.met_lochadtopo().met_forward().x());
    float met_muonboy_x = (event.met_muonboy().x());
    float met_refmuon_x = (event.met_refmuon_track().x());

    float met_central_y = (event.met_lochadtopo().met_central().y());
    float met_endcap_y = (event.met_lochadtopo().met_endcap().y());
    float met_forward_y = (event.met_lochadtopo().met_forward().y());
    float met_muonboy_y = (event.met_muonboy().y());
    float met_refmuon_y = (event.met_refmuon_track().y());

    float met_x = ((met_central_x + met_endcap_x + met_forward_x) - met_refmuon_x) + met_muonboy_x;
    float met_y = ((met_central_y + met_endcap_y + met_forward_y) - met_refmuon_y) + met_muonboy_y;
    
    ALorentzVector met = alv(met_x, met_y);

    auto all_jets = make_plist<AJet>(event.pb.jets_antikt4h1topoem());
    auto all_muons = make_plist<AMuon>(event.pb.muons_staco())
    auto all_electrons = make_plist<AElectron>(event.pb.electrons())

    ///////////// MUON SELECTION ////////////////

    auto mcf = cutflow("muon/cutflow")
    #define MUON_CUT(nr, condition) {LIST_CUT(muons,condition); PASSED_CUT(m_cf, nr, 1.0);}

    plist<AMuon> cosmic_candidates;

    plist<AMuon> muons;
    foreach(ptr<AMuon> m, all_muons) {
        if (!data && do_mu_smearing) {
            m_scl->SetSeed(event_number, m->pb->index(), 680049);
            m_scl->Event(m->ms.pt(),m->track.pt(),m->lv.pt(),m->lv.eta());

            double smear = (m_scl->pTCB()/m->lv.pt());
            if (m->lv.pt()) m->lv *= smear;
            if (m->ms.pt()) m->ms *= (m_scl->pTMS()/m->ms.pt());
            if (m->track.pt()) m->track *= (m_scl->pTID()/m->track.pt());

            // Set Mass to 0 (TODO: to compare with christian)
            m->lv.E = sqrt(m->lv.p2());

            m->met_contribution.px = (1. - (1./smear)) * m->lv.px;
            m->met_contribution.py = (1. - (1./smear)) * m->lv.py;
        }

        mcf->cut("initial", true);
        MUON_CUT("initial", true);
        MUON_CUT("tight", m->pb->tight()); 
        MUON_CUT("combined", m->pb->combined());
            S<H1>("muon/eta", 100, -5, 5).fill(m->lv.eta());
        MUON_CUT("eta", fabs(m->lv.eta()) < 2.4)
            S<H1>("muon/pt", 400, 0, 400).fill(m->lv.pt()/GeV);
        MUON_CUT("pt", m->lv.pt() > 15*GeV);

        TrackHits hits = m->pb->track_hits();
            S<H1>("muon/blayer", 10, 0, 10).fill(hits.numberofblayerhits());
        MUON_CUT("blayer", (!hits.expectblayerhit()) || hits.numberofblayerhits() >= 1)
            S<H1>("muon/pixel", 20, 0, 20).fill(hits.numberofpixelhits());
        MUON_CUT("pixel", hits.numberofpixelhits() + hits.numberofpixeldeadsensors() > 1)
            S<H1>("muon/pixel", 50, 0, 50).fill(hits.numberofscthits());
        MUON_CUT("sct", hits.numberofscthits() + hits.numberofsctdeadsensors() >= 6)
            S<H1>("muon/si_holes", 50, 0, 50).fill(hits.numberofpixelholes() + hits.numberofsctholes());
        MUON_CUT("si_holes", hits.numberofpixelholes() + hits.numberofsctholes() < 3)

        int outliers = hits.numberoftrtoutliers();
        int n = hits.numberoftrthits() + outliers;
        bool good_trt;
        if (fabs(m->lv.eta()) < 1.9) good_trt = ((n > 5) && (outliers < 0.9*n));
        else good_trt = ((n <= 5) || (outliers < 0.9 * n));
            S<H1>("muon/trt_hits", 100, 0, 100).fill(hits.numberoftrthits());
        MUON_CUT("trt", good_trt);

        // Do not require z0 / d0 for cosmic candidates
        if (m->pb->isolation().etcone20()/m->lv.pt() < 0.15 && m->pb->isolation().ptcone20()/m->lv.pt() < 0.1)
            cosmic_candidates.push_back(m);

            S<H1>("muon/z0", 20000, -1000, 1000).fill(m->pb->perigee_id().z0());
        MUON_CUT("z0", fabs(m->pb->perigee_id().z0()) < 10*mm);
            S<H1>("muon/d0", 2000, -10, 10).fill(m->pb->perigee_id().d0());
        MUON_CUT("d0", fabs(m->pb->perigee_id().d0()/m->pb->perigee_id().d0err()) < 10)
            S<H1>("muon/etiso_rel", 200, 0, 10).fill(m->pb->isolation().etcone20()/m->lv.pt());
        MUON_CUT("etiso", m->pb->isolation().etcone20()/m->lv.pt() < 0.15)
            S<H1>("muon/ptiso_rel", 200, 0, 10).fill(m->pb->isolation().ptcone20()/m->lv.pt());
        MUON_CUT("ptiso", m->pb->isolation().ptcone20()/m->lv.pt() < 0.1)

        if (!data && do_mu_sf) {
            muon_scale_factor *= m_staco_cb_sf->scaleFactor(TLV(m->lv));
            // scaleFactorUncertainty
        }
        plot_kinematics_1p("muon/selected/", m->lv, event_scale_factor);
        muons.push_back(muon)
    }

    ///////////// ELECTRON SELECTION ////////////////

    if (!data) e_rsc->SetRandomSeed(event_number);

    CUTFLOW(e_cf,"electron/cutflow")
    #define ELECTRON_CUT(nr, condition) {LIST_CUT(electrons,condition); PASSED_CUT(e_cf,nr,1.0);}
    #define ELECTRON_RCUT(nr, condition) {LIST_RCUT(electrons,condition); PASSED_CUT(e_cf,nr,1.0);}

    vector<AElectronPtr> lar_hole_electrons; // all electrons with pt>15 GeV

    LOOP(AElectronPtr, e, electrons)
        // Smear or correct electrons
        double smear = 1.0;
        if (data) {
            double e_new = GeV*e_rsc->applyEnergyCorrection(e->cluster.eta(), e->cluster.phi(), e->cluster.e()/GeV, e->ET()/GeV, 0, "ELECTRON");
            smear = e_new/e->cluster.e();
            S<H1>("electron/correction_factor",100, 0.9, 1.1).fill(e_new/e->cluster.e());
        } else if (do_el_smearing) {
            smear = e_rsc->getSmearingCorrection(e->cluster.eta(), e->cluster.e()/GeV, 0, true);
            S<H1>("electron/smearing_factor",100,0.5,1.5).fill(smear);
        };
        double target_e = smear * e->cluster.e();
        e->lv = (target_e/e->track.p()) * e->track;
        e->lv.E = target_e;
        e->cluster *= smear;
        e->track   *= smear;

        e->met_contribution.px = (1. - (1./smear)) * e->lv.px;
        e->met_contribution.py = (1. - (1./smear)) * e->lv.py;

        // Check if there is a container electron with > 15GeV pointing at the LAR hole
        if (e->ET() > 15*GeV) {
            lar_hole_electrons.push_back(e);
            if (3 == m_checkOQ->checkOQClusterElectron(data ? run_number : 180614, e->cluster.eta(), e->cluster.phi()))
                electron_pointing_at_hole = true;
        }

        // Electron cuts proper
        ELECTRON_CUT("initial", true);
            S<H1>("electron/author",100,0,100).fill(e->pb->author());
        ELECTRON_CUT("author", e->pb->author() == 1 || e->pb->author() == 3)
            S<H1>("electron/et", 200, 0, 200).fill(e->ET()/GeV);
        ELECTRON_CUT("ET", e->ET() > 15*GeV)
            S<H1>("electron/cl_eta", 100, -5, 5).fill(e->cluster.eta());
        ELECTRON_CUT("eta", fabs(e->cluster.eta()) < 2.47 && !e->in_crack())
        ELECTRON_CUT("OQ", !e->pb->bad_oq())
        ELECTRON_CUT("tight", e->pb->tight())
            S<H1>("electron/z0", 2000, -100, 100).fill(e->pb->perigee().z0());
        ELECTRON_CUT("z0", fabs(e->pb->perigee().z0()) < 10*mm)
            S<H1>("electron/d0", 2000, -10, 10).fill(e->pb->perigee().d0());
        ELECTRON_CUT("d0", fabs(e->pb->perigee().d0()/e->pb->perigee().d0err()) < 10)
            S<H1>("electron/etiso_rel", 200, 0, 10).fill(e->pb->isolation().etcone20()/e->ET());
        ELECTRON_CUT("etiso", e->pb->isolation().etcone20()/e->ET() < 0.15)
            S<H1>("electron/ptiso_rel", 200, 0, 10).fill(e->pb->isolation().ptcone20()/e->ET());
        ELECTRON_CUT("ptiso", e->pb->isolation().ptcone20()/e->ET() < 0.1)
            
        double closest = 10;
        foreach(AMuonPtr m, muons) closest = min(closest, m->track.delta_r(e->track));
            S<H1>("electron/dR_muon", 200, 0, 10).fill(closest);
        ELECTRON_CUT("e-m OR", closest > 0.1);

    END_LOOP

    // Electron-Electron OR
    RLOOP(AElectronPtr, e, electrons)
        bool overlap = false;
        foreach(AElectronPtr re, electrons) if (e != re && 0.1 > re->cluster.delta_r(e->cluster)) overlap = true;
        ELECTRON_RCUT("e-e OR", !overlap);
        plot_kinematics_1p("electron/selected/", e->lv, event_scale_factor);
    END_LOOP

    // Electron scale factor 
    if (!data && do_el_sf) {
        LOOP(AElectronPtr, e, electrons)
            double eta = e->cluster.eta(); // using this since this seems to be used by the sf makers
            pair<float,float> sf_r = e_sf->scaleFactorRecoTrkQual(eta);
            pair<float,float> sf_t = e_sf->scaleFactorTightETcorrected(eta, e->lv.et(), 3);
            if (sf_r.first > 0) electron_scale_factor *= sf_r.first*sf_t.first;
            //electron_scale_factor_esq += pow2(sf_r.second) + pow2(sf_t.second)
        END_LOOP
    }

    ///////////// JET SELECTION ////////////////

    // Check if there is a jet pointing at the LAR hole
    bool jet_pointing_at_hole = false;
    if (!data || run_number >= 180614) {
        foreach(AJetPtr j, jets) {
            bool overlap = false;
            foreach(AElectronPtr re, lar_hole_electrons) {
                double deltaeta = j->lv.eta() - re->lv.eta();
                double deltaphi = fabs(j->lv.phi() - re->lv.phi());
                if (deltaphi > M_PI) deltaphi = 2*M_PI - deltaphi;
                float delta_r = sqrt(deltaeta * deltaeta + deltaphi*deltaphi); 
                //float delta_r = e->cluster.delta_r(j->lv);
                if (0.3 >= delta_r) {
                    overlap = true; 
                    break; 
                }

            }
            if (!overlap)
                if (LArHole::IsLArHoleVeto(j->lv.pt(), j->lv.eta(), j->lv.phi(), j->pb->bch_corr_jet(), j->pb->bch_corr_cell(), data, 25000.0))
                    jet_pointing_at_hole = true;
        }
    }

    // Check if there is a bad jet in the event (recalculate badness)
    bool bad_jet_in_event = false;
    if (data) {
        foreach(AJetPtr j, jets) {
            if (j->lv.pt() > 20*GeV && j->is_bad()) {
                bool overlap = false;
                foreach(AElectronPtr e, electrons) {
                double deltaeta = j->lv.eta() - e->lv.eta();
                double deltaphi = fabs(j->lv.phi() - e->lv.phi());
                if (deltaphi > M_PI) deltaphi = 2*M_PI - deltaphi;
                double delta_r = sqrt(deltaeta * deltaeta + deltaphi*deltaphi); 
                    //float delta_r = e->cluster.delta_r(j->lv);
                    if (0.3 > delta_r) {
                        overlap = true; 
                        break; 
                    }
                }
                if (overlap) continue;
                foreach(AMuonPtr m, muons) if (m->lv.pt() > 15*GeV &&  0.3 > m->lv.delta_r(j->lv)) { overlap = true; break; }
                if (!overlap) bad_jet_in_event = true;
            }
        }
    };

    // Jet selection proper
    CUTFLOW(j_cf,"jet/cutflow")
    #define JET_CUT(nr, condition) {LIST_CUT(jets,condition); PASSED_CUT(j_cf,nr,1.0);}
    #define JET_RCUT(nr, condition) {LIST_RCUT(jets,condition); PASSED_CUT(j_cf,nr,1.0);}
    LOOP(AJetPtr, j, jets)
        JET_CUT("initial", true)
        JET_CUT("pt", j->lv.pt() > 25*GeV)
        JET_CUT("eta", fabs(j->lv.eta()) < 4.5)
        //JET_CUT("jvf", !(fabs(j->lv_em.eta()) < 2.1 && j->pb->jet_vertex_fraction() < 0.75))
        bool overlap = false;
        foreach(AElectronPtr re, electrons) if (0.3 > re->cluster.delta_r(j->lv)) overlap = true;
        JET_CUT("j-e OR", !overlap);
        plot_kinematics_1p("jet/selected/", j->lv);
    END_LOOP
    
    ///////////// EVENT SELECTION ////////////////
    // Create Channel Histograms 
    auto cf = S<Cutflow>("cutflow");
    cf.add_scaling("mc", event.mc_event_weight());
    cf.add_scaling("sf", event_scale_factor);
    #define PASSED(cut) {c1.passed(cut);}

    PASSED("initial")

    // Data: Remove stream overlap if required
    if (data && remove_overlap) { 
        foreach(int s, event.stream_tag()) 
            if (s == Muons) 
                return;
    }
    PASSED("overlap")

    // Data: Check if GRL passed
    if (data && !(grl->pass(run_number, event.lumi_block()))) 
        return;
    PASSED("GRL")

    // MC: reweight due to pileup
    if (!data && do_pileup_reweighting) {
        double pileup_weight = prw->getPileupWeight(event.lumi_block());
        if (pileup_weight < 0) cerr << "ERROR: Pileup weight error " << pileup_weight << endl;
        event_scale_factor *= pileup_weight;
        S<H1>("pileup_weight",100,0,10).fill(pileup_weight);
    }
    PASSED("w_{pileup}") // Bookkeeping


    // ----- INTERMEZZO ------
    // MC & Data: MET correction due to selected leptons
    foreach(AElectronPtr e, electrons) met -= e->met_contribution;
    foreach(AMuonPtr m, muons) met -= m->met_contribution;

    // now put leptons into leptons
    std::vector<ALepton> lepton;
    foreach(AElectronPtr e, electrons) lepton.push_back(ALepton(*e));
    foreach(AMuonPtr m, muons) lepton.push_back(ALepton(*m));
    std::sort(lepton.begin(), lepton.end());

    // Calculate met_rel
    double _min_dphi = 100;
    foreach(AElectronPtr o, electrons) _min_dphi = min(fabs(o->lv.delta_phi(met)), _min_dphi);
    foreach(AMuonPtr o, muons) _min_dphi = min(fabs(o->lv.delta_phi(met)), _min_dphi);
    foreach(AJetPtr o, jets) _min_dphi = min(fabs(o->lv.delta_phi(met)), _min_dphi);
    double met_rel = (_min_dphi < M_PI/2) ? met.pt()*sin(_min_dphi) : met.pt();
    
    // Some Histograms
    S<H1>("muon/scale_factor")(200,0,2).fill(muon_scale_factor);
    S<H1>("electron/scale_factor")(200,0,2).fill(electron_scale_factor, event_scale_factor);
    S<H1>("initial/lb",100,0,100).fill(event.lumi_block(), event_scale_factor);
    S<H1>("initial/lar_hole_scale_factor",10,0,2).fill(event_scale_factor, event_scale_factor);
    S<H1>("initial/mc_weight",8,-2,2).fill(event.mc_event_weight(), event_scale_factor);
    S<H1>("initial/n_jets", 30, 0, 30).fill(jets.size(), event_scale_factor);
    // ----- END INTERMEZZO ------

    // Skim all events with at least two good leptons:
    if (lepton.size() >= 2) event_passed(event);

    // INITIAL PLOTTING + COSMIC SECTION
    S<H1>("met/et", 1000,0,1000).fill(met.pt()/GeV);
    S<H1>("met/phi", 1000,-M_PI, M_PI).fill(met.phi());
    S<H1>("met/px", 1000,0,1000).fill(met.px/GeV);
    S<H1>("met/py", 1000,0,1000).fill(met.py/GeV);

    // Cosmic half-muon checks - require one cosmic candidate & one other lepton
    if (cosmic_candidates.size() + electrons.size() >= 2) { 
        vector<double> leptons_z0;
        foreach(AMuonPtr l, cosmic_candidates) leptons_z0.push_back(l->pb->perigee_id().z0() + event.vertices(l->pb->vertex_index()).z());
        foreach(AElectronPtr l, electrons) leptons_z0.push_back(l->pb->perigee().z0() + event.vertices(l->pb->vertex_index()).z());
        if (leptons_z0.size()==2) {
            HIST1_FILL("cosmics/2l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
        } else if (leptons_z0.size()==3) {
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[2]), event_scale_factor);
        } else if (leptons_z0.size()==4) {
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[3]), event_scale_factor);
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[3]), event_scale_factor);
            HIST1_FILL("cosmics/3l_dz0",1000,0,1000,fabs(leptons_z0[2]-leptons_z0[3]), event_scale_factor);
        } else {
            foreach(double z0_1, leptons_z0) 
                foreach(double z0_2, leptons_z0) 
                    HIST1_FILL("cosmics/4lplus_dz0",1000,0,1000,fabs(z0_1-z0_2), event_scale_factor);
        }
    }
    if (muons.size() + electrons.size() >= 2) { 
        vector<double> leptons_z0;
        foreach(AMuonPtr l, muons) leptons_z0.push_back(l->pb->perigee_id().z0() + event.vertices(l->pb->vertex_index()).z());
        foreach(AElectronPtr l, electrons) leptons_z0.push_back(l->pb->perigee().z0() + event.vertices(l->pb->vertex_index()).z());
        if (leptons_z0.size()==2) {
            HIST1_FILL("cosmics_withz0/2l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
        } else if (leptons_z0.size()==3) {
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[2]), event_scale_factor);
        } else if (leptons_z0.size()==4) {
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[3]), event_scale_factor);
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[3]), event_scale_factor);
            HIST1_FILL("cosmics_withz0/3l_dz0",1000,0,1000,fabs(leptons_z0[2]-leptons_z0[3]), event_scale_factor);
        } else {
            foreach(double z0_1, leptons_z0) 
                foreach(double z0_2, leptons_z0) 
                    HIST1_FILL("cosmics_withz0/4lplus_dz0",1000,0,1000,fabs(z0_1-z0_2), event_scale_factor);
        }
    }
    if (muons.size() + electrons.size() >= 2 && event.vertices_size() > 0 && event.vertices(0).tracks() >= 3) {
        vector<double> leptons_z0;
        foreach(AMuonPtr l, muons) leptons_z0.push_back(l->pb->perigee_id().z0() + event.vertices(l->pb->vertex_index()).z());
        foreach(AElectronPtr l, electrons) leptons_z0.push_back(l->pb->perigee().z0() + event.vertices(l->pb->vertex_index()).z());
        if (leptons_z0.size()==2) {
            HIST1_FILL("cosmics_withz0pv0/2l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
        } else if (leptons_z0.size()==3) {
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[2]), event_scale_factor);
        } else if (leptons_z0.size()==4) {
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[1]), event_scale_factor);
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[0]-leptons_z0[3]), event_scale_factor);
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[2]), event_scale_factor);
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[1]-leptons_z0[3]), event_scale_factor);
            HIST1_FILL("cosmics_withz0pv0/3l_dz0",1000,0,1000,fabs(leptons_z0[2]-leptons_z0[3]), event_scale_factor);
        } else {
            foreach(double z0_1, leptons_z0) 
                foreach(double z0_2, leptons_z0) 
                    HIST1_FILL("cosmics_withz0pv0/4lplus_dz0",1000,0,1000,fabs(z0_1-z0_2), event_scale_factor);
        }
    }

    if (event.vertices().size() == 1) {
        bool empty_trigger = true;
        foreach(Trigger t, event.triggers()) {
            if (t.fired() && t.name() != Trigger::EF_mu20_empty) empty_trigger = false;
        }
        if (empty_trigger) {
            S<H1>("cosmics/met_et", 1000,0,1000).fill(met.pt()/GeV);

            if (cosmic_candidates.size() >= 1) {
                AMuonPtr m0 = cosmic_candidates[0];
                ALorentzVector l0 = m0->lv;
                if (met.pt() > 40*GeV) {
                    HIST1_FILL("cosmics/met_gt40_phi", 1000,-M_PI, M_PI, met.phi(), event_scale_factor);
                    HIST2_FILL("cosmics/met_gt40_phi_lphi", 100,-M_PI, M_PI, 100, -M_PI,M_PI, met.phi(), l0.phi(), event_scale_factor);
                }

                plot_kinematics_1p(_results.get(), "cosmics/single_muon_", l0, event_scale_factor);
                HIST1_FILL("cosmics/single_muon_z0", 1000, -500, 500, m0->pb->perigee_id().z0(), event_scale_factor);

                if (cosmic_candidates.size() >= 2) {
                    AMuonPtr m1 = cosmic_candidates[1];
                    ALorentzVector l1 = m1->lv;
                    double d_phi = l0.delta_phi(l1) - M_PI;
                    if (d_phi < -M_PI) d_phi += 2*M_PI;

                    HIST1_FILL("cosmics/mm_sum_eta", 1000, -10, 10, l0.eta()+l1.eta(), event_scale_factor)
                    HIST1_FILL("cosmics/mm_sum_eta_fine", 1000, -0.1, 0.1, l0.eta()+l1.eta(), event_scale_factor)
                    HIST1_FILL("cosmics/mm_dphi_opp", 1000, -M_PI, M_PI, d_phi, event_scale_factor)
                    HIST1_FILL("cosmics/mm_dphi_opp_fine", 1000, -0.1, 0.1, d_phi, event_scale_factor)
                    HIST2_FILL("cosmics/mm_sum_eta_dphi_opp",  100, -10, 10, 100, -M_PI, M_PI, l0.eta()+l1.eta(),d_phi, event_scale_factor)
                    HIST2_FILL("cosmics/mm_sum_eta_dphi_opp_fine",  200, -0.1, 0.1, 200, -0.1, 0.1, l0.eta()+l1.eta(),d_phi, event_scale_factor)
                    HIST2_FILL("cosmics/mm_sum_eta_dphi_opp_ultrafine",  200, -0.01, 0.01, 200, -0.01, 0.01, l0.eta()+l1.eta(), d_phi, event_scale_factor)
                }
            }
        }
    }
    HIST1_FILL("n_vtx", 30, 0, 30, event.vertices().size(), event_scale_factor);
    // End Cosmic section
    
    // Cut: Three tracks at PV0
    if (event.vertices_size() == 0 || event.vertices(0).tracks() < 3)
        return;
    PASSED("PV_{0}")

    // Cosmic section II: Require any trigger other than "empty"
    bool empty_trigger = true;
    foreach(Trigger t, event.triggers()) {
        if (t.fired() && t.name() != Trigger::EF_mu20_empty) empty_trigger = false;
    }
    if (!empty_trigger && cosmic_candidates.size() >= 2) {
        AMuonPtr m0 = cosmic_candidates[0];
        AMuonPtr m1 = cosmic_candidates[1];
        ALorentzVector l0 = m0->lv;
        ALorentzVector l1 = m1->lv;
        double d_phi = l0.delta_phi(l1) - M_PI;
        if (d_phi < -M_PI) d_phi += 2*M_PI;

        HIST1_FILL("cosmic_check_nocut/mm_sum_eta", 1000, -10, 10, l0.eta()+l1.eta(), event_scale_factor)
        HIST1_FILL("cosmic_check_nocut/mm_sum_eta_fine", 1000, -0.1, 0.1, l0.eta()+l1.eta(), event_scale_factor)
        HIST1_FILL("cosmic_check_nocut/mm_dphi_opp", 1000, -M_PI, M_PI, d_phi, event_scale_factor)
        HIST1_FILL("cosmic_check_nocut/mm_dphi_opp_fine", 1000, -0.1, 0.1, d_phi, event_scale_factor)
        HIST2_FILL("cosmic_check_nocut/mm_sum_eta_dphi_opp",  100, -10, 10, 100, -M_PI, M_PI, l0.eta()+l1.eta(),d_phi, event_scale_factor)
        HIST2_FILL("cosmic_check_nocut/mm_sum_eta_dphi_opp_fine",  200, -0.1, 0.1, 200, -0.1, 0.1, l0.eta()+l1.eta(),d_phi, event_scale_factor)
        HIST2_FILL("cosmic_check_nocut/mm_sum_eta_dphi_opp_ultrafine",  200, -0.01, 0.01, 200, -0.01, 0.01, l0.eta()+l1.eta(), d_phi, event_scale_factor)

        double z0_0 = m0->pb->perigee_id().z0() + event.vertices(m0->pb->vertex_index()).z();
        double z0_1 = m1->pb->perigee_id().z0() + event.vertices(m1->pb->vertex_index()).z();
        double z0_err = add_in_sq(m0->pb->perigee_id().z0err(), m1->pb->perigee_id().z0err());
        double d0_0 = m0->pb->perigee_id().d0();
        double d0_1 = m1->pb->perigee_id().d0();

        HIST1_FILL("cosmic_check_nocut/d_z0", 10000, 0, 1000, fabs(z0_0-z0_1), 1.0);
        HIST1_FILL("cosmic_check_nocut/d_d0", 10000, 0, 1000, fabs(d0_0-d0_1), 1.0);
        HIST1_FILL("cosmic_check_nocut/d_z0_rel", 1000, 0, 1000, fabs(z0_0-z0_1)/z0_err, 1.0);
        if (met.pt() > 40*GeV) {
            HIST1_FILL("cosmic_check_nocut/met_gt40_phi", 1000,-M_PI, M_PI, met.phi(), event_scale_factor);
            HIST2_FILL("cosmic_check_nocut/met_gt40_phi_l0phi", 100,-M_PI, M_PI, 100, -M_PI,M_PI, met.phi(), l0.phi(), event_scale_factor);
            HIST2_FILL("cosmic_check_nocut/met_gt40_phi_l1phi", 100,-M_PI, M_PI, 100, -M_PI,M_PI, met.phi(), l1.phi(), event_scale_factor);
        }
    }

    if (data) {
        if (electron_pointing_at_hole) return;
        PASSED("FEB_electron")
        if (jet_pointing_at_hole) return;
        PASSED("FEB_jet")
    }

    // Data/MC: Cut / Reweight if LAr Hole is hit
    if (electron_pointing_at_hole || jet_pointing_at_hole) {
        if (data) return;
        else event_scale_factor *= no_lar_hole_lumi_fraction;
    }
    PASSED("FEB") // for bookkeeping
    
    // Data: MET cleaning
    if (data && bad_jet_in_event) return;
    PASSED("MET clean")

    // Cut: Trigger
    bool trigger_e = false;
    bool trigger_m = false;
    foreach(Trigger t, event.triggers()) { 
        if (t.fired()) {
            if (t.name() == Trigger::EF_e20_medium) trigger_e = true;
            if (t.name() == Trigger::EF_mu18_MG) trigger_m = true;
            if (t.name() == Trigger::EF_mu40_MSonly_barrel) trigger_m = true;
        }
    }

    if (!trigger_e) { ee = false; ssee = false; }
    if (!trigger_m) { mm = false; ssmm = false; }
    if (!trigger_e && !trigger_m) { em = false; ssem = false; }
    PASSED("Trigger")

    S<H1>("triggered/n_good_leptons")(20,0,20).fill(electrons.size() + muons.size(), event_scale_factor);
    S<H1>("triggered/n_good_electrons")(20,0,20).fill(electrons.size(), event_scale_factor);
    S<H1>("triggered/n_good_muons")(20,0,20).fill(muons.size(), event_scale_factor);

    // Plot kinematics
    ALorentzVector l0 = lepton[0].lv;
    ALorentzVector l1 = lepton[1].lv;
    ALorentzVector ll = l0 + l1;

    // Cut: exactly two leptons
    if (electrons.size() != 2 || muons.size() != 0) {
        add_channel("ee");
        add_channel("ee/", (lepton[0].charge != lepton[1].charge) ? "os" : "ss");
    } else if (electrons.size() != 1 || muons.size() != 1) {
        add_channel("em");
        add_channel("em/", (lepton[0].charge != lepton[1].charge) ? "os" : "ss");
    } else if (electrons.size() != 0 || muons.size() != 2) {
        add_channel("mm");
        add_channel("mm/", (lepton[0].charge != lepton[1].charge) ? "os" : "ss");
    }
    PASSED("2 leptons")

    // only plot same sign stuff in same sign plots
    if (lepton[0].charge != lepton[1].charge) 
        set_channel("sign", "os")
    else
        set_channel("sign", "ss")

    KINEMATIC_PLOTS("presel_2l")

    // Cosmic section III - check for survivors
    if (mm && cosmic_candidates.size() >= 2) {
        AMuonPtr m0 = cosmic_candidates[0];
        AMuonPtr m1 = cosmic_candidates[1];
        ALorentzVector l0 = m0->lv;
        ALorentzVector l1 = m1->lv;
        double d_phi = l0.delta_phi(l1) - M_PI;
        if (d_phi < -M_PI) d_phi += 2*M_PI;

        HIST1_FILL("cosmic_check/mm_sum_eta", 1000, -10, 10, l0.eta()+l1.eta(), event_scale_factor)
        HIST1_FILL("cosmic_check/mm_sum_eta_fine", 1000, -0.1, 0.1, l0.eta()+l1.eta(), event_scale_factor)
        HIST1_FILL("cosmic_check/mm_dphi_opp", 1000, -M_PI, M_PI, d_phi, event_scale_factor)
        HIST1_FILL("cosmic_check/mm_dphi_opp_fine", 1000, -0.1, 0.1, d_phi, event_scale_factor)
        HIST2_FILL("cosmic_check/mm_sum_eta_dphi_opp",  100, -10, 10, 100, -M_PI, M_PI, l0.eta()+l1.eta(),d_phi, event_scale_factor)
        HIST2_FILL("cosmic_check/mm_sum_eta_dphi_opp_fine",  200, -0.1, 0.1, 200, -0.1, 0.1, l0.eta()+l1.eta(),d_phi, event_scale_factor)
        HIST2_FILL("cosmic_check/mm_sum_eta_dphi_opp_ultrafine",  200, -0.01, 0.01, 200, -0.01, 0.01, l0.eta()+l1.eta(), d_phi, event_scale_factor)

        double z0_0 = m0->pb->perigee_id().z0() + event.vertices(m0->pb->vertex_index()).z();
        double z0_1 = m1->pb->perigee_id().z0() + event.vertices(m1->pb->vertex_index()).z();
        double z0_err = add_in_sq(m0->pb->perigee_id().z0err(), m1->pb->perigee_id().z0err());
        double d0_0 = m0->pb->perigee_id().d0();
        double d0_1 = m1->pb->perigee_id().d0();
        HIST1_FILL("cosmic_check/d_z0", 10000, 0, 1000, fabs(z0_0-z0_1), 1.0);
        HIST1_FILL("cosmic_check/d_d0", 10000, 0, 1000, fabs(d0_0-d0_1), 1.0);
        HIST1_FILL("cosmic_check/d_z0_rel", 1000, 0, 1000, fabs(z0_0-z0_1)/z0_err, 1.0);
        
        if (met.pt() > 40*GeV) {
            HIST1_FILL("cosmic_check/met_gt40_phi", 1000,-M_PI, M_PI, met.phi(), event_scale_factor);
            HIST2_FILL("cosmic_check/met_gt40_phi_l0phi", 100,-M_PI, M_PI, 100, -M_PI,M_PI, met.phi(), l0.phi(), event_scale_factor);
            HIST2_FILL("cosmic_check/met_gt40_phi_l1phi", 100,-M_PI, M_PI, 100, -M_PI,M_PI, met.phi(), l1.phi(), event_scale_factor);
        }
    }

    // Cut: First lepton > 25 GeV
    if (!(l0.pt() > 25*GeV)) return;
    PASSED("p_{T,1} > 25 GeV")

    KINEMATIC_PLOTS("presel_pt1_gt25")

    // Cut: Trigger match
    vector<AElectronPtr> el_trigger_matched;
    foreach(AElectronPtr e, electrons) 
        if (e->lv.et() > 25*GeV) {
            foreach(int n, e->pb->matched_trigger()) 
                if (n == Trigger::EF_e20_medium) {el_trigger_matched.push_back(e); break;}
            /*foreach(Trigger t, event.triggers()) {
                if (t.fired() && t.name() == Trigger::EF_e20_medium) {
                    foreach(TriggerFeature f, t.features_trig_electron()) {
                        if (e->lv.delta_r(f.eta(), f.phi()) < 0.15) {
                            el_trigger_matched.push_back(e); break;
                        }
                    }
                }
            }*/
        }

    vector<AMuonPtr> mu_trigger_matched;
    foreach(AMuonPtr m, muons) {
        bool matched = false;
        foreach(int n, m->pb->matched_trigger_efi_mg()) {
            if (n == Trigger::EF_mu18_MG && m->lv.pt() > 20*GeV) { mu_trigger_matched.push_back(m); matched = true; break; }
        }
        if (!matched) foreach(int n, m->pb->matched_trigger_efi_cb()) {
            if (n == Trigger::EF_mu40_MSonly_barrel && m->lv.pt() > 42*GeV) { mu_trigger_matched.push_back(m); break; }
        }
    }

    if (do_trigger_match) {
        if (mu_trigger_matched.size() + el_trigger_matched.size() == 0) return;
    }
    PASSED("trigger match")

    KINEMATIC_PLOTS("presel_trigmatch")

    // MC: Apply trigger SF
    if (!data && do_trigger_sf) {
        vector<double> trigger_sff;
        vector<double> trigger_eff;
        foreach(AElectronPtr e, el_trigger_matched) {
            trigger_sff.push_back(e_sf->scaleFactorTrigger(e->cluster.eta()).first);
            trigger_eff.push_back(0.9894);
        }
        foreach(AMuonPtr m, mu_trigger_matched) {
            const bool combined = false;
            //cout << m_tsf->get_sf(m->lv.pt(), m->lv.eta(), m->lv.phi(), combined) << " eff " << m_tsf->get_mc_eff(m->lv.pt(), m->lv.eta(), m->lv.phi(), combined) << endl;
            trigger_sff.push_back(m_tsf->get_sf(m->lv.pt(), m->lv.eta(), m->lv.phi(), combined));
            trigger_eff.push_back(m_tsf->get_mc_eff(m->lv.pt(), m->lv.eta(), m->lv.phi(), combined));
        }
        trigger_sff.push_back(0); trigger_eff.push_back(0); // is used if no second lepton trigger
        if (trigger_eff[0] == 0 && trigger_eff[1] == 0 && trigger_sff[0] == 0 && trigger_sff[1] == 0) return;
        
        //cout << trigger_eff[0] << " , " << trigger_eff[1] << " , " << trigger_sff[0] << " , " << trigger_sff[1] << " == " << trigger_event_sf(trigger_eff[0], trigger_eff[1], trigger_sff[0], trigger_sff[1]) << " ?? " << mu_trigger_matched.size() + el_trigger_matched.size() << endl;
        event_scale_factor *= trigger_event_sf(trigger_eff[0], trigger_eff[1], trigger_sff[0], trigger_sff[1]);

        if (isnan(event_scale_factor) || event_scale_factor != event_scale_factor) { // ISNAN TODO: Is this the right thing to do???
            foreach(AMuonPtr m, mu_trigger_matched) 
                cout << m->lv.pt() << " , " << m->lv.eta() << " , " << m->lv.phi() << endl;
            cout << trigger_eff[0] << " , " << trigger_eff[1] << " , " << trigger_sff[0] << " , " << trigger_sff[1] << endl;
            cout << trigger_event_sf(trigger_eff[0], trigger_eff[1], trigger_sff[0], trigger_sff[1]) << endl; 
            cout << event_scale_factor << endl;
            return;
        }
    }
    PASSED("trigger sf") // for bookkeeping

    event_scale_factor *= muon_scale_factor;
    PASSED("muon sf") // for bookkeeping

    event_scale_factor *= electron_scale_factor;
    PASSED("electron sf") // for bookkeeping

    // Cut: Check for same sign
    if (lepton[0].charge == lepton[1].charge) {
        os = false;
        em = false;
        ee = false;
        mm = false;
    } else {
        ss = false;
        ssem = false;
        ssee = false;
        ssmm = false;
    }
    PASSED("opposite sign")

    KINEMATIC_PLOTS("presel_sign")

    // ANALYSIS CUTS FOR 2L OS:

    bool same_flavor = (lepton[0].flavor == lepton[1].flavor);
    HIST1_FILL("cut_plots/1_low_llm",200,0,200,ll.m()/GeV, event_scale_factor);
    if (ll.m() <= (same_flavor ? 15*GeV : 10*GeV)) return;
    PASSED("X < m_{ll}")

    KINEMATIC_PLOTS("past_1_low_llm")

    HIST1_FILL("cut_plots/2_z_veto",200,0,200,ll.m()/GeV, event_scale_factor);
    if (same_flavor && fabs(ll.m() - m_Z) <= 15*GeV) return;
    PASSED("Z veto")

    KINEMATIC_PLOTS("past_2_zveto")
 
    HIST1_FILL("cut_plots/3_met_rel",200,0,200,met_rel/GeV, event_scale_factor);
    if (met_rel <= (same_flavor ? 40*GeV : 25*GeV)) return;
    PASSED("MET_{rel}")

    KINEMATIC_PLOTS("past_3_metrel")

    // 0 jet analysis for now
    HIST1_FILL("cut_plots/4_njetsl",30,0,30,jets.size(),event_scale_factor);
    if (jets.size() != 0) return;
    PASSED("0 jets")

    KINEMATIC_PLOTS("past_4_0jets")
    
    HIST1_FILL("cut_plots/5_ll_pt",400,0,200,ll.pt()/GeV,event_scale_factor);
    if (ll.pt() <= 30*GeV) return;
    PASSED("p_{T,ll} > 30 GeV")
    
    KINEMATIC_PLOTS("past_5_llpt")

    //  Mll < 50 (65) GeV for low (high) mass 
    HIST1_FILL("cut_plots/6_ll_m",400,0,400,ll.m()/GeV,event_scale_factor);
    if (ll.m() >= 50*GeV) return;
    PASSED("m_{ll} < 50 GeV")

    KINEMATIC_PLOTS("past_6_llm")

    double ll_dphi = fabs(lepton[0].lv.delta_phi(lepton[1].lv));
    HIST1_FILL("cut_plots/7_dphi_ll",864,0,M_PI,ll_dphi,event_scale_factor);
    if (ll_dphi >= 1.3) return;
    PASSED("#Delta #phi_{ll} < 1.3")

    KINEMATIC_PLOTS("past_7_dphi")

    double mt_higgs = add_in_sq(ll.mt() + met.pt(), (ll + met).pt());
    HIST1_FILL("cut_plots/8_mt_higgs",1000,0,1000,mt_higgs/GeV,event_scale_factor);

    CUT_ORDER("m_{T,Higgs120}")
    CUT_ORDER("m_{T,Higgs130}")
    CUT_ORDER("m_{T,Higgs140}")
    CUT_ORDER("m_{T,Higgs150}")
    CUT_ORDER("m_{T,Higgs160}")
    CUT_ORDER("m_{T,Higgs170}")
    CUT_ORDER("m_{T,Higgs180}")
    CUT_ORDER("m_{T,Higgs190}")
    CUT_ORDER("m_{T,Higgs200}")

    #define HIGGSCUT(mass, str) \
    if (.75*GeV*(mass) < mt_higgs && mt_higgs < GeV*(mass)) {\
        PASSED("m_{T,Higgs" str "}");\
        KINEMATIC_PLOTS("m_{T,Higgs" str "}");\
    }
    HIGGSCUT(120,"120");
    HIGGSCUT(130,"130");
    HIGGSCUT(140,"140");
    HIGGSCUT(150,"150");
    HIGGSCUT(160,"160");
    HIGGSCUT(170,"170");
    HIGGSCUT(180,"180");
    HIGGSCUT(190,"190");
    HIGGSCUT(200,"200");
};


class HWWAnalysisConfiguration : public ConfigurationOf<HWWAnalysis> {
  public:
    GRLPtr grl;
    string grl_name;
    bool data, remove_overlap;
    double no_lar_hole_lumi_fraction, lumi_pb, m_higgs;
    bool do_mu_smearing, do_el_smearing, do_trigger_sf, do_mu_sf, do_el_sf, do_pileup_reweighting, do_trigger_match;

    virtual boost::program_options get_options() {
        boost::program_options opt;
        opt.add("lumi", po::value<double>(&lumi_pb)->default_value(1024.0), "lumi [pb]");
        opt.add("data", po::value<bool>(&data)->default_value(false), "set if data");
        opt.add("remove-overlap", po::value<bool>(&remove_overlap)->default_value(false), "drop egamma stream events");
        opt.add("grl", po::value<string>(&grl_name), "GRL file");
        return opt;
    }

    bool initialize(ConfigArguments &arguments) {
        if (arguments.count("grl")) grl = new GRL(arguments["grl"].as<string>());
        no_lar_hole_lumi_fraction = double(11.2683 + (double)152.2343)/lumi_pb;
        do_pileup_reweighting = true;
        do_mu_smearing = true;
        do_el_smearing = true;
        do_mu_sf = true;
        do_el_sf = true;
        do_trigger_match = true;
        do_trigger_sf = true;
        m_higgs = 160*GeV;
        return true;
    }

    bool configure(HWWAnalysis &a) {
        a.prw = new Root::TPileupReweighting();
        int sc = a.prw->initialize("ilumicalc_histograms_EF_mu18_MG_178044-184169.root", "avgintperbx", "mu_mc10b.root", "mu_mc10b");
        if (sc) {
            cerr << "ERROR: TPileupReweighting initialize failed, StatusCode " << sc << endl;
            return false;
        }
        a.e_rsc = new EnergyRescaler();
        a.e_rsc->useDefaultCalibConstants("2011");
        a.m_scl = new SmearingClass("muid"); // STACO
        a.m_scl->UseScale(1);
        a.m_checkOQ = new egammaOQ();
        a.m_staco_cb_sf = new Analysis::StacoCBScaleEffFactors();
        a.m_staco_tight_sf = new Analysis::StacoTightScaleEffFactors();
        a.e_sf = new egammaSFclass();
        a.m_tsf = new MuonTriggerSF("SF_for_EPS.root");
        return true;
    }
};

//a4_application(JobConfiguration<HWWAnalysis>);
a4_application(HWWAnalysisConfiguration);

