#include <iostream>

#include <boost/program_options.hpp>
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
#include "a4/root/atlas/ntup_smwz/Event.pb.h"

#include "a4/root/test/Event.pb.h"


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
    // TBranch::ResetBit(kDoNotProcess) in the message factory.
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
    namespace po = boost::program_options;

    std::string tree_name, tree_type, output_file;
    std::vector<std::string> input_files;
    Long64_t event_count = -1;

    po::positional_options_description p;
    p.add("input", -1);

    po::options_description commandline_options("Allowed options");
    commandline_options.add_options()
        ("help,h", "produce help message")
        ("tree-name,n", po::value<std::string>(&tree_name), "input TTree name")
        ("tree-type,T", po::value<std::string>(&tree_type)->default_value("test"), "which event factory to use (SMWZ, PHOTON, test)")
        ("input,i", po::value<std::vector<std::string> >(&input_files), "input file names")
        ("output,o", po::value<std::string>(&output_file)->default_value("test_io.a4"), "output file name")
        ("event-count,c", po::value<Long64_t>(&event_count)->default_value(-1), "number of events to process (-1=all available)")
    ;
    
    po::variables_map arguments;
    po::store(po::command_line_parser(argc, argv).
              options(commandline_options).positional(p).run(), arguments);
    po::notify(arguments);
    
    if (arguments.count("help") || !arguments.count("input"))
    {
        std::cout << "Usage: " << argv[0] << " [Options] input(s)" << std::endl;
        std::cout << commandline_options << std::endl;
        return 1;
    }

    TChain input(tree_name.c_str());
    
    foreach (const std::string& input_file, input_files)
        input.Add(input_file.c_str());

    a4::io::A4Output a4o(output_file, "Event");
    shared<a4::io::OutputStream> stream = a4o.get_stream(); 
    
    if (tree_type == "test")
        copy_tree(input, stream, a4::root::test::Event::descriptor(), event_count);
    else if (tree_type == "PHOTON")
        copy_tree(input, stream, a4::root::atlas::ntup_photon::Event::descriptor(), event_count);
    else if (tree_type == "SMWZ")
        copy_tree(input, stream, a4::root::atlas::ntup_smwz::Event::descriptor(), event_count);
    else
        // TODO(pwaller): Accept .proto files here
        throw a4::Fatal("Unknown filetype '", tree_type, "', should be test, PHOTON or SMWZ.");
}


