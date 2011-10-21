#include <iostream>
#include <a4/application.h>
#include <a4/histogram.h>
#include <a4/cutflow.h>
#include <a4/alorentzvector.h>
#include <a4/root/atlas/ntup_smwz/Event.pb.h>

using namespace std;
using namespace a4::process;
using namespace a4::hist;
using namespace a4::root::atlas::ntup_smwz;

class MyProcessor : public ProcessorOf<Event> {
  public:
    virtual void process(const Event & event) {
        //cout << event.event_number() << endl;
        Cutflow & cf = S.T<Cutflow>("main_cutflow")();
        cf.weight(1.2);
        cf.passed("initial");
        S.T<H1>("test")(100,0,10).fill(2.1);
        if (event.mu_staco_size() > 0) {
            ALorentzVector v = ALorentzVector::from(event.mu_staco(0));
            kinematic_plots(S("leading_mu_"),v);
            cf.passed("muon");
        }
        write(event);

    }

    void kinematic_plots(ObjectStore D, ALorentzVector v) {
        D.T<H1>("pT")(200,0,200).fill(v.pt());
        D.T<H1>("eta")(40,-5,5).fill(v.eta());
        D.T<H1>("phi")(100,-M_PI,M_PI).fill(v.eta());
        D.T<H1>("m")(100,0,100).fill(v.m());
    }
};

int main(int argc, const char * argv[]) {
    return a4_main_process<MyProcessor>(argc, argv);
};
