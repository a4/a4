#include <iostream>

#include <boost/function.hpp>
#include <boost/bind.hpp>
//using namespace boost;
using boost::bind;
using boost::function;

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
using google::protobuf::Message;
using google::protobuf::Descriptor;

#include <TChain.h>

#include <a4/output.h>
#include <a4/input.h>

#include "common.h"

#include "a4/root/atlas/ntup_photon/Event.pb.h"
#include "a4/root/atlas/ntup_photon/Metadata.pb.h"

#include "a4/root/test/Event.pb.h"

A4RegisterClass(a4::root::test::Event);
A4RegisterClass(a4::root::atlas::ntup_photon::Event);
A4RegisterClass(a4::root::atlas::ntup_photon::Metadata);


/// Builds a RootToMessageFactory when Notify() is called.
class EventFactoryBuilder : public TObject
{
TTree& _tree;
const Descriptor* _descriptor;
RootToMessageFactory* _factory;

public:

    EventFactoryBuilder(TTree& t, const Descriptor* d, RootToMessageFactory* f) :
        _tree(t), _descriptor(d), _factory(f)
    {}

    /// Called when the TTree branch addresses change. 
    /// Generates a new message factory for the _tree.
    Bool_t Notify() 
    { 
        assert(_descriptor);
        //std::cout << "Notify start" << std::endl;
        (*_factory) = make_message_factory(&_tree, _descriptor);
        //std::cout << "Notify end" << std::endl;
        return true;
    }
};

/// Copies `tree` into the `stream` using information taken from the compiled in
/// Event class.
void copy_tree(TTree& tree, shared<a4::io::OutputStream> stream, 
    const Descriptor* message_descriptor, Long64_t entries = -1)
{
    Long64_t tree_entries = tree.GetEntries();
    if (entries > tree_entries)
        entries = tree_entries;
    if (entries < 0)
        entries = tree_entries;
        
    std::cout << "Will process " << entries << " entries" << std::endl;
    
    // Nothing to do!
    if (!entries)
        return;
    
    // Disable all branches. Branches get enabled through 
    // TBranch::ResetBit(kDoNotProcess) we make the message factory.
    tree.SetBranchStatus("*", false);
    
    // An event_factory is automatically created when the branch pointers change
    // through the Tree Notify() call.
    RootToMessageFactory event_factory;
    
    // This is the only place where we say that we're wanting to build the 
    // Event class.
    EventFactoryBuilder builder(tree, message_descriptor, &event_factory);
    
    tree.SetNotify(&builder);
    // This line is needed. It seems to sometimes not get called automatically 
    // depending on the underlying TTree.
    builder.Notify();
    
    size_t total_bytes_read = 0;
    
    for (Long64_t i = 0; i < entries; i++)
    {
        //std::cout << "Reading event " << i << std::endl;
        size_t read_data = tree.GetEntry(i);
        total_bytes_read += read_data;
        if (i % 100 == 0)
            std::cout << "Progress " << i << " / " << entries << " (" << read_data << ")" << std::endl;
        
        // Write out one event.
        stream->write(*event_factory());
    }
    
    //Metadata m;
    //m.set_total_events(entries);
    //stream->metadata(m);
    
    std::cout << "Copied " << entries << " entries (" << total_bytes_read << ")" << std::endl;
}

int main(int argc, char ** argv) {
    a4::io::A4Output a4o("test_io.a4", "Event");

    shared<a4::io::OutputStream> stream = a4o.get_stream(); 

    TChain input("photon");
    input.Add("input/*.root*");
    ///a4::root::atlas::ntup_photon::Event
    // a4::root::test::Event
    copy_tree(input, stream, a4::root::atlas::ntup_photon::Event::descriptor(), 2000);
}


