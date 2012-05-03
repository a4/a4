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

class MyProcessor : public ProcessorOf<Event> {
  public:
    virtual void process(const Event& event) {
        //cout << event.event_number() << endl;
        Cutflow& cf = S.T<Cutflow>("main_cutflow")("Main Cutflow");
        cf.weight(1.2);
        cf.passed("Initial");
        S.T<H1>("test")("Test Title")(100,0,10,"Test X axis").fill(2.1);
        if (event.mu_staco_size() > 0) {
            ALorentzVector v = ALorentzVector::from(event.mu_staco(0));
            kinematic_plots(S("leading_muon/"),v);
            cf.passed("Muon Cut");
        }
        write(event);

    }

    void kinematic_plots(ObjectStore D, ALorentzVector v) {
        D.T<H1>("pT")(200,0,200,"p_T [GeV]").fill(v.pt());
        D.T<H1>("eta")(40,-5,5).fill(v.eta());
        D.T<H1>("phi")(100,-M_PI,M_PI).fill(v.eta());
        D.T<H2>("eta_phi")("Eta-Phi")(20,-5,5,"#eta")(60,-M_PI,M_PI,"#phi").fill(v.eta(), v.phi());
        D.T<H1>("m")(100,0,100).fill(v.m());
    }
};

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
