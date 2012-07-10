#include <iostream>
#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/alorentzvector.h>
// !!!! NOTE !!!!
// This tutorial works with ntup-smwz event. You need to install a4 with
// ./waf configure --prefix $HOME/bin/a4 --enable-a4-python --enable-atlas-ntup=smwz
// ./waf build install
// Otherwise you could only work with standard a4/atlas/Event.pb.h 
// To get an overview of the smwz event attributes you should look into
// "a4atlas/proto/a4/atlas/ntup/smwz/Event.proto" in your a4 folder
// The Attributes for other objects e.g. Muon are in 
// "a4atlas/proto/a4/atlas/ntup/smwz/Muon.proto"
#include <a4/atlas/ntup/smwz/Event.pb.h>

using namespace std;
using namespace a4::process;
using namespace a4::hist;
// same here without the waf configure option: --enable-atlas-ntup=smwz 
// otherwise you can only use the standard a4::atlas;
using namespace a4::atlas::ntup::smwz;

const double GeV = 1000.0;
void kinematic_plots(ObjectStore D, ALorentzVector v) {
    D.T<H1>("pT")(200,0,200,"p_T [GeV]").fill(v.pt()/GeV);
    D.T<H1>("eta")(40,-5,5).fill(v.eta());
    D.T<H1>("phi")(100,-M_PI,M_PI).fill(v.eta());
    D.T<H2>("eta_phi")("Eta-Phi")(20,-5,5,"#eta")(60,-M_PI,M_PI,"#phi").fill(v.eta(), v.phi());
    D.T<H1>("m")(150,0,150).fill(v.m());
}

class MyProcessor : public ProcessorOf<Event> {
public:
    virtual void process(const Event& event) {
        // The basic object is the socalled ObjectStore. The idea about 
        // the ObjectStore is, that you can access objects by name quickly.
        // e.g. creating Cutflows and Histograms is very simple. Here are some
        // examples what you can do with the ObjectStore:
        Cutflow& cf = S.T<Cutflow>("main_cutflow")("Main Cutflow");

        // some weighting
        cf.weight(1.2);

        // adds this event to the cutflow with label "Initial"
        cf.passed("Initial");

        // create a histogram as follow:
        S.T<H1>("test")("Test Title")(100,0,10,"Test X axis").fill(2.1);
        // <H1>                     one dimensional hist
        // ("test")                 name of histogram. every '/' is interpreted in a folder structure
        // ("Test Title")           title
        // (100,0,10,"Name X axis") (number of bins, from, to, ... )
        // .fill(2.1)               fills the histogram with numbers. 

        // another example a histogram with the number of muons in a event is created like:
        S.T<H1>("n_muons")("Number of muons")(5,0,5).fill(event.mu_staco_size());

        // the kinematics plots for the muons:
        if (event.mu_staco_size()>0) {
            foreach(auto muon, event.mu_staco()){
                ALorentzVector mu_lv = ALorentzVector::from(muon);
                kinematic_plots(S("muons/"),mu_lv);
            }
            cf.passed("Past Muon Cut");
        }

        //  kinematics plots for electrons splited by their event occurrence number
        if (event.el_size() > 0) {
            S.T<H1>("n_electrons")("Number of electrons")(20,0,20).fill(event.el_size());
            for (int i=0;i<event.el_size();++i){
                // aquire lorentz vector of electron via pt, eta, phie, e:
                ALorentzVector e_lv = ALorentzVector::from_ptetaphie(event.el(i).pt(), event.el(i).eta(), event.el(i).phi(), event.el(i).e());
                //ALorentzVector e_lv = ALorentzVector::from(event.el(i));
                kinematic_plots(S("electron/",i,"/"), e_lv);
                S.T<H1>("electron/",i,"/")("Transverse Energie")(500,0,100).fill(event.el(i).et()/GeV);
                // NOTE: Every attribute with an uppercase letter is only available with downcase letters.
             }
        }

        // aquire missing et
        RefFinalMET met = event.met_reffinal();
        // change the ObjectStore path to create an control region
        if (met.et() > 50*GeV) S=S("high_missing_et/");
        // from now on every object you create, will be in the folder
        // "high_missing_et" for events with more then 50 GeV of M_Et.
        S.T<H1>("missing_et")(500,0,100).fill(met.et()/GeV);
    }
};

// WORKFLOW:
// At first you will need a file for your anlysis.
// Try to get e.g. "data12_8TeV.00200987.physics_Muons.merge.NTUP_SMWZ.f437_m1126_p1067_p1068_tid00845060_00/NTUP_SMWZ.00845060._000032.root.1"
// over dq2.
// -convert it to a4:
// root2a4 -n 10000 -t physics -T smwz.proto -i NTUP_SMWZ.00845060._000032.root.1 -o NTUP_SMWZ.00845060._000032.root.1.a4
// (NOTE: the smwz.proto must be in your folder where you use root2a4. just copy it from the a4 directory.)
// to get root file in the end with all histograms, etc in it your further steps will be:
// - compile tutorial with make
// - execute your analysis:
// ./analysis -i YOUR_NTUP_SMWZ_FILE.a4 -r result.a4
// (NOTE: converting root file in a4 format: use root2a4 or look into doc/faq)
// - You can convert your resultfile to root now:
// a4results2root -t1 -R result.root result.a4
// Now you should be able to open the result.root file in the normal TBrowser
// (NOTE: if you have  many histos and cutflows spereated by the simulation or run number
//  you can use a4merge to merge all of them. Try:
//  a4merge a4merge -t1 --per simulation result.a4 -r result_merged.a4
//  or:
//  a4merge -t1 --per run result.a4 -r result_merged.a4 )
// There are also the "plot" and "cutflow" scripts
// usage: plot result.root -p n_electrons

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
