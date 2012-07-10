#include <iostream>
#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/alorentzvector.h>
#include <a4/atlas/ntup/smwz/Event.pb.h>

using namespace std;
using namespace a4::process;
using namespace a4::hist;
using namespace a4::atlas::ntup::smwz;

const double GeV = 1000.0;
void kinematic_plots(ObjectStore D, ALorentzVector v) {
    D.T<H1>("pT")(200,0,200,"p_T [GeV]").fill(v.pt()/GeV);
    D.T<H1>("eta")(40,-5,5).fill(v.eta());
    D.T<H1>("phi")(100,-M_PI,M_PI).fill(v.eta());
    D.T<H2>("eta_phi")("Eta-Phi")(20,-5,5,"#eta")(60,-M_PI,M_PI,"#phi").fill(v.eta(), v.phi());
    D.T<H1>("m")(150,0,150).fill(v.m());
}
bool sort_pt(const ALorentzVector &v1, const ALorentzVector &v2){
    return v1.pt() < v2.pt();
};

class MyProcessor : public ProcessorOf<Event> {
  public:
    virtual void process(const Event& event) {
        // if systematics are called the hole event will be proccessed
        // in the normal way and once for each systematic you call
        // note: you need to call systematic at beginning of the process function
        systematic("Lower_Cut");
        systematic("Higher_Cut");
        S.T<Cutflow>("cutflow")("Main Cutflow");
        S.T<Cutflow>("cutflow").passed("Initial");

        // only events with at least one electron for now
        if (event.el_size() == 0) return;
        std::vector<ALorentzVector> electrons;

        // aquire lorentzvector from electrons and sorting
        foreach( auto electron,  event.el())  electrons.push_back(ALorentzVector::from(electron));
        std::sort(electrons.begin(), electrons.end(), sort_pt);
        ALorentzVector el_v = electrons[0];
        std::vector<ALorentzVector> muons;

        // aquire muons ,sorting and plotting
        S.T<H1>("n_muons")("Number of muons")(5,0,5).fill(event.mu_staco_size());
        if (event.mu_staco_size()> 0){
            foreach( auto muon,  event.mu_staco()){ 
                muons.push_back(ALorentzVector::from(muon));
                kinematic_plots(S("muons/"),ALorentzVector::from(muon));
            }
            std::sort(muons.begin(), muons.end(), sort_pt);
            ALorentzVector mu_v = muons[0];
            kinematic_plots(S("leading_muon/"),mu_v);
        }

        // kinematic histograms for the leading lepton
        kinematic_plots(S("initial_leading_electron/"),el_v);
        S.T<Cutflow>("cutflow").passed("electron cut");

        // in_systematic("name") returns true when it is proccessing this systematic channel
        double pt_cut = 40;
        if (in_systematic("Lower_Cut")) pt_cut = 30;
        else if (in_systematic("Higher_Cut")) pt_cut = 50;
        if (el_v.pt() > pt_cut * GeV) return;
        kinematic_plots(S("past_electron_pt_cut/"),el_v);
        S.T<Cutflow>("cutflow").passed("leading electron pt Cut > ",pt_cut,"GeV");

        // do again the proceeding as described in tutorial 02 to get to your resultroot file.
        // after that you will have the normal and two systematic folders in your resultfile.
        // now you can use the cutflow script with the option '-y Higher_Cut,Lower_Cut' 
        // to see the systematics in your cutflow:
        // cutflow  your_root_result_file.root --latex -p cutflow -y Higher_Cut,Lower_Cut
    }
};

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
