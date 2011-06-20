/**
 * ProtoBuf Results container: all histograms are kept in the H1 form
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#include <iostream>

#include "H1.h"
#include "PBResults.h"

using namespace std;

pb::Results::Results():
    ::Results(PROTO_BUF)
{
    _jets.reset(new H1(20, 0, 20));
    _jet_flavor.reset(new H1(10, 0, 10));
    _jet_e.reset(new H1(100, 100, 300));
    _jet_px.reset(new H1(100, 0, 100));
    _jet_py.reset(new H1(100, 0, 100));
    _jet_pz.reset(new H1(100, 0, 100));
    _jet_x.reset(new H1(20, 0, 20));
    _jet_y.reset(new H1(20, 0, 20));
    _jet_z.reset(new H1(20, 0, 20));

    _muons.reset(new H1(20, 0, 20));
    _muon_e.reset(new H1(100, 100, 300));
    _muon_px.reset(new H1(50, 50, 150));
    _muon_py.reset(new H1(50, 50, 150));
    _muon_pz.reset(new H1(50, 50, 150));
    _muon_x.reset(new H1(20, -10, 10));
    _muon_y.reset(new H1(20, -10, 10));
    _muon_z.reset(new H1(20, -10, 10));
}

pb::Results::~Results()
{
}

void pb::Results::add(const ::Results &results)
{    
    if (PROTO_BUF != results.type())
        return;

    const Results source = dynamic_cast<const Results &>(results);

    // Add results
    //
    _jets->add(*source._jets);
    _jet_flavor->add(*source._jet_flavor);
    _jet_e->add(*source._jet_e);
    _jet_px->add(*source._jet_px);
    _jet_py->add(*source._jet_py);
    _jet_pz->add(*source._jet_pz);
    _jet_x->add(*source._jet_x);
    _jet_y->add(*source._jet_y);
    _jet_z->add(*source._jet_z);

    _muons->add(*source._muons);
    _muon_e->add(*source._muon_e);
    _muon_px->add(*source._muon_px);
    _muon_py->add(*source._muon_py);
    _muon_pz->add(*source._muon_pz);
    _muon_x->add(*source._muon_x);
    _muon_y->add(*source._muon_y);
    _muon_z->add(*source._muon_z);
}

void pb::Results::print() const
{
    cout << " [ JETS ]" << endl;
    cout << *_jets << endl;
    cout << *_jet_flavor << endl;
    cout << *_jet_e << endl;
    cout << *_jet_px << endl;
    cout << *_jet_py << endl;
    cout << *_jet_pz << endl;
    cout << *_jet_x << endl;
    cout << *_jet_y << endl;
    cout << *_jet_z << endl;
    cout << endl;
    cout << " [ MUONS ]" << endl;
    cout << *_muons << endl;
    cout << *_muon_e << endl;
    cout << *_muon_px << endl;
    cout << *_muon_py << endl;
    cout << *_muon_pz << endl;
    cout << *_muon_x << endl;
    cout << *_muon_y << endl;
    cout << *_muon_z << endl;
}


pb::Results::H1Ptr pb::Results::jets() const
{
    return _jets;
}

pb::Results::H1Ptr pb::Results::jet_flavor() const
{
    return _jet_flavor;
}

pb::Results::H1Ptr pb::Results::jet_e() const
{
    return _jet_e;
}

pb::Results::H1Ptr pb::Results::jet_px() const
{
    return _jet_px;
}

pb::Results::H1Ptr pb::Results::jet_py() const
{
    return _jet_py;
}

pb::Results::H1Ptr pb::Results::jet_pz() const
{
    return _jet_pz;
}

pb::Results::H1Ptr pb::Results::jet_x() const
{
    return _jet_x;
}

pb::Results::H1Ptr pb::Results::jet_y() const
{
    return _jet_y;
}

pb::Results::H1Ptr pb::Results::jet_z() const
{
    return _jet_z;
}

pb::Results::H1Ptr pb::Results::muons() const
{
    return _muons;
}

pb::Results::H1Ptr pb::Results::muon_e() const
{
    return _muon_e;
}

pb::Results::H1Ptr pb::Results::muon_px() const
{
    return _muon_px;
}

pb::Results::H1Ptr pb::Results::muon_py() const
{
    return _muon_py;
}

pb::Results::H1Ptr pb::Results::muon_pz() const
{
    return _muon_pz;
}

pb::Results::H1Ptr pb::Results::muon_x() const
{
    return _muon_x;
}

pb::Results::H1Ptr pb::Results::muon_y() const
{
    return _muon_y;
}

pb::Results::H1Ptr pb::Results::muon_z() const
{
    return _muon_z;
}
