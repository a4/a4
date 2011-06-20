/**
 * Process ProtoBuf file: read events
 *
 * Created by Samvel Khalatyan on Mar 8, 2011
 * Copyright 2011, All rights reserved
 */

#include "a4/H1.h"
#include "a4/processor.h"
#include "a4/results.h"
#include "a4/reader.h"

#include "pb/Event.pb.h"

#include "instructor.h"


Processor::Processor():
    _events_read(0),
    _events_read_in_last_file(0)
{
    _results.reset(new Results());
}

void Processor::init(const fs::path &file)
{
    _reader.reset(new Reader(file));
    _events_read_in_last_file = 0;
}

void Processor::processEvents()
{
    if (!_reader)
        return;

    process();

    _events_read_in_last_file = _reader->eventsRead();
    _events_read += _events_read_in_last_file;

    _reader.reset();
}

void Processor::process()
{
    Event event;

    while(_reader->good() && _reader->read_event(event)) {

        const int number_of_jets = event.jets_size();
        HIST1(_results,"jets",20,0,20)->fill(number_of_jets);

        for(int i = 0; i < number_of_jets; ++i)
        {
            const Jet &jet = event.jets(i);
            HIST1(_results,"jet_e",100,0,100000)->fill(jet.p4().e());
            HIST1(_results,"jet_px",100,0,100000)->fill(jet.p4().px());
            HIST1(_results,"jet_py",100,0,100000)->fill(jet.p4().py());
            HIST1(_results,"jet_pz",100,0,100000)->fill(jet.p4().pz());
        }

        const int number_of_muons = event.muons_size();
        _results->h1("muons", 20, 0, 20)->fill(number_of_muons);

        for(int i = 0; i < number_of_muons; ++i)
        {
            const Muon &muon = event.muons(i);
            HIST1(_results,"muon_e",100,0,100000)->fill(muon.p4().e());
            HIST1(_results,"muon_px",100,0,100000)->fill(muon.p4().px());
            HIST1(_results,"muon_py",100,0,100000)->fill(muon.p4().py());
            HIST1(_results,"muon_pz",100,0,100000)->fill(muon.p4().pz());
        }
    }
}

ResultsPtr Processor::results() const
{
    return _results;
}

uint32_t Processor::eventsRead() const
{
    return _events_read;
}

uint32_t Processor::eventsReadInLastFile() const
{
    return _events_read_in_last_file;
}

ProcessingJob::ProcessingJob() : _threads(-1) {};

bool ProcessingJob::process_files(ProcessingJob::Inputs inputs) {
    if (_threads != -1)
    {
        boost::shared_ptr<Instructor> instructor(new Instructor(this, _threads));
        instructor->processFiles(inputs);
        _results = instructor->results();
    }
    else
    {
        boost::shared_ptr<Processor> processor;
        processor = get_processor();
        for(Inputs::const_iterator input = inputs.begin();
            inputs.end() != input;
            ++input)
        {
            fs::path file_path(*input);
            processor->init(file_path);
            processor->processEvents();
        }
        _results = processor->results();
    }
    return true;
}
