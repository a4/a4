
#include "example.h"

#include "a4/application.h"
#include "a4/results.h"
#include "pb/Event.pb.h"

void Example::process_event(Event & event)
{
    const int number_of_jets = event.jets_size();
    HIST1("jets",20,0,20)->fill(number_of_jets);

    for(int i = 0; i < number_of_jets; ++i)
    {
        const Jet &jet = event.jets(i);
        HIST1("jet_e",100,0,100000)->fill(jet.p4().e());
        HIST1("jet_px",100,0,100000)->fill(jet.p4().px());
        HIST1("jet_py",100,0,100000)->fill(jet.p4().py());
        HIST1("jet_pz",100,0,100000)->fill(jet.p4().pz());
    }

    const int number_of_muons = event.muons_size();
    _results->h1("muons", 20, 0, 20)->fill(number_of_muons);

    for(int i = 0; i < number_of_muons; ++i)
    {
        const Muon &muon = event.muons(i);
        HIST1("muon_e",100,0,100000)->fill(muon.p4().e());
        HIST1("muon_px",100,0,100000)->fill(muon.p4().px());
        HIST1("muon_py",100,0,100000)->fill(muon.p4().py());
        HIST1("muon_pz",100,0,100000)->fill(muon.p4().pz());
    }
}


int main(int argc, char ** argv) {
    ExampleJob job;
    int rv = a4_main(argc, argv, job);
    if (rv != 0) return rv;
    job.results()->print();
}
