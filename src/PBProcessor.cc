/**
 * Process ProtoBuf file: read events
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#include "H1.h"
#include "PBProcessor.h"
#include "PBReader.h"
#include "Results.h"
#include "pb/Event.pb.h"

pb::Processor::Processor():
    _events_read(0),
    _events_read_in_last_file(0)
{
    _results.reset(new Results());
}

void pb::Processor::init(const fs::path &file)
{
    _reader.reset(new Reader(file));
    _events_read_in_last_file = 0;
}

void pb::Processor::processEvents()
{
    if (!_reader)
        return;

    process();

    _events_read_in_last_file = _reader->eventsRead();
    _events_read += _events_read_in_last_file;

    _reader.reset();
}

void pb::Processor::process()
{
    Event event;

    while(_reader->good() && _reader->read_event(event)) {

        const int number_of_jets = event.jets_size();
        _results->h1("jets",20,0,20)->fill(number_of_jets);

        for(int i = 0; i < number_of_jets; ++i)
        {
            const Jet &jet = event.jets(i);

            _results->h1("jet_e",100,0,100000)->fill(jet.p4().e());
            _results->h1("jet_px",100,0,100000)->fill(jet.p4().px());
            _results->h1("jet_py",100,0,100000)->fill(jet.p4().py());
            _results->h1("jet_pz",100,0,100000)->fill(jet.p4().pz());
        }

        const int number_of_muons = event.muons_size();
        _results->h1("muons", 20, 0, 20)->fill(number_of_muons);

        for(int i = 0; i < number_of_muons; ++i)
        {
            const Lepton &muon = event.muons(i);

            _results->h1("muon_e",100,0,100000)->fill(muon.p4().e());
            _results->h1("muon_px",100,0,100000)->fill(muon.p4().px());
            _results->h1("muon_py",100,0,100000)->fill(muon.p4().py());
            _results->h1("muon_pz",100,0,100000)->fill(muon.p4().pz());
        }
    }
}

pb::Processor::ResultsPtr pb::Processor::results() const
{
    return _results;
}

uint32_t pb::Processor::eventsRead() const
{
    return _events_read;
}

uint32_t pb::Processor::eventsReadInLastFile() const
{
    return _events_read_in_last_file;
}
