#include <iostream>
#include <fstream>
using std::ifstream;
#include <iterator>
using std::istream_iterator;

#include <boost/bind.hpp>
using boost::bind;

#include <boost/filesystem.hpp>
using boost::filesystem::current_path;
#include <boost/program_options.hpp>

#include <boost/chrono.hpp>
namespace chrono = boost::chrono;
typedef chrono::duration<double> duration;

#include <boost/tokenizer.hpp>
using boost::tokenizer;
using boost::char_separator;


#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
using google::protobuf::Message;
using google::protobuf::MessageFactory;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Reflection;
using google::protobuf::FileDescriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::DescriptorPoolDatabase;
using google::protobuf::MergedDescriptorDatabase;
using google::protobuf::compiler::MultiFileErrorCollector;
using google::protobuf::compiler::DiskSourceTree;
using google::protobuf::compiler::SourceTreeDescriptorDatabase;

#include <TFile.h>
#include <TChain.h>
#include <TROOT.h>
#include <RVersion.h>

#include <a4/types.h>
#include <a4/output.h>
#include <a4/output_stream.h>
#include <a4/input.h>

#include <a4/root/common.h>
#include <a4/root/period_mapping.h>

//#include <a4/atlas/ntup/photon/Event.pb.h>
//#include <a4/atlas/ntup/smwz/Event.pb.h>

#include <a4/root/EventMetaData.pb.h>
using a4::atlas::EventMetaData;
using a4::atlas::ProcessingStep;
using a4::atlas::InputFile;


typedef std::vector<shared<Message> > MessageBuffer;
typedef boost::function<void (shared<a4::io::OutputStream>, const MessageBuffer&)> MetadataCallback;
typedef boost::function<void ()> FlushCallback;

class MetadataFactory
{
protected:
    chrono::steady_clock::time_point _start_wallclock;
#ifdef BOOST_CHRONO_HAS_THREAD_CLOCK
    chrono::thread_clock::time_point _start_cpuclock;
#endif
    
    const Descriptor* _desc;
    const Reflection* _refl;
    const FieldDescriptor* _field_run;
    const FieldDescriptor* _field_mc_channel;
    
    uint32_t _run_number, _mc_channel;
    
    MetadataFactory(const MetadataFactory&);
    void operator=(const MetadataFactory&);
    
public:    
    EventMetaData _metadata;
    ProcessingStep* _processing_step;
    
    uint64_t _input_events, _input_bytes_read;
    
    MetadataFactory() 
        : _desc(NULL), _refl(NULL), _field_run(NULL), _field_mc_channel(NULL), 
          _run_number(0), _mc_channel(0)
    {}
    
    virtual void reset_counters()
    {
        _start_wallclock = chrono::steady_clock::now();
        _start_cpuclock = chrono::thread_clock::now();
        
        _input_events = _input_bytes_read = 0;
        
        _metadata.Clear();
        _processing_step = _metadata.add_processing_steps();
    }
    
    void populate_fields(const shared<Message>& event)
    {
        if (_desc && _refl)
            return;
        _desc = event->GetDescriptor();
        _refl = event->GetReflection();
        _field_run = _desc->FindFieldByName("run_number");
        _field_mc_channel = _desc->FindFieldByName("mc_channel_number");
        
    }
    
    // Check to see if any vital properties have changed since we last looked
    virtual bool need_new_metadata(shared<Message> event)
    {
        populate_fields(event);
        if (_field_run && _refl->HasField(*event, _field_run)) {
            const uint32_t run_number = _refl->GetUInt32(*event, _field_run);
            if (run_number != _run_number) {
                _run_number = run_number;
                if (_input_events > 0)
                    return true;
            }
        }
        
        if (_field_mc_channel && _refl->HasField(*event, _field_mc_channel)) {
            const uint32_t mc_channel = _refl->GetUInt32(*event, _field_mc_channel);
            if (mc_channel != _mc_channel) {
                _mc_channel = mc_channel;
                if (_input_events > 0)
                    return true;
            }
        }
        
        return false;
    }
    
    // Called when the metadata object is about to be written
    // FIll the fields on the metadata
    virtual void compute_info(const MessageBuffer& buffer)
    {
        ProcessingStep& p = *_processing_step;
        
        p.set_name("root2a4");
    
        p.set_walltime((chrono::steady_clock::now() - _start_wallclock).count());
        p.set_cputime((chrono::thread_clock::now() - _start_cpuclock).count());
        
        p.set_input_events(_input_events);
        p.set_input_bytes_read(_input_bytes_read);
        
        _metadata.set_event_count(buffer.size());
    }
        
    void write_metadata(shared<a4::io::OutputStream> stream, 
                         const MessageBuffer& buffer)
    {
        compute_info(buffer);
        // Only write metadata if there is at least one message to be written
        // (A4 doesn't deal well with the case of metadata for 0 events yet)
        if (buffer.size() >= 1) {
            stream->metadata(_metadata);
        }
        reset_counters();
    }
};

class D3PDMetadataFactory : public MetadataFactory
{
    TChain& _chain;
    
    typedef MetadataFactory super;
    
    D3PDMetadataFactory(const D3PDMetadataFactory&);
    void operator=(const D3PDMetadataFactory&);
    
public:
    D3PDMetadataFactory(TChain& chain) 
        : _chain(chain)
    {
        reset_counters();
    }
    
    virtual void reset_counters()
    {
        super::reset_counters();
        
        // The input filename is known at the point the counters are reset.
        InputFile& i = *_processing_step->add_input_files();
        
        TFile* input_file = _chain.GetFile();
        TTree* input_tree = _chain.GetTree();
        
        i.set_filename(input_file->GetName());
        i.set_size(input_file->GetSize());
        i.set_first_entry_index(input_tree->GetReadEntry());
        
        std::string grl_path = std::string("Lumi/") + _chain.GetName();
        TObjString* grl_string = dynamic_cast<TObjString*>(input_file->Get(grl_path.c_str()));
        
        if (grl_string) {
            i.set_grl_string(grl_string->GetString().Data());
        } else
        {
            WARNING("Couldn't get GRL string..");
        }
    }
    
    void compute_info(const MessageBuffer& buffer)
    {   
        super::compute_info(buffer);
        
        if (buffer.size() >= 1) {
            if (_field_run) {
                auto run_number = _refl->GetUInt32(*buffer[0], _field_run);
                _metadata.add_run(run_number);
                
                auto* full_period = get_period(run_number);
                std::string period = full_period;
                
                if (period != "UNK") {
                    period = period[0];
                }
                
                _metadata.add_period(period);
                _metadata.add_subperiod(full_period);
            }
            if (_field_mc_channel && _refl->HasField(*buffer[0], _field_mc_channel)) 
                _metadata.add_mc_channel(_refl->GetUInt32(*buffer[0], _field_mc_channel));
        }
    }
};

/// Builds a RootToMessageFactory when Notify() is called.
class EventFactoryBuilder : public TObject
{
    TTree& _tree;
    const Descriptor* _descriptor;
    RootToMessageFactory* _factory;
    MessageFactory* _dynamic_factory;
    FlushCallback _flush_callback;
    
    bool _brand_new;

public:

    /// 
    EventFactoryBuilder(TTree& t, const Descriptor* d, RootToMessageFactory* f, 
                        MessageFactory* dynamic_factory, 
                        FlushCallback flush_callback)
        : _tree(t), _descriptor(d), _factory(f), 
          _dynamic_factory(dynamic_factory), _flush_callback(flush_callback),
          _brand_new(true)
    {
    }

    /// Called when the TTree branch addresses change. 
    /// Generates a new message factory for the _tree.
    Bool_t Notify() 
    { 
        assert(_descriptor);
        INFO("Processing new file: ", _tree.GetCurrentFile()->GetName());
        if (!_brand_new) {
            _flush_callback();
        }
        _brand_new = false;
        //std::cout << "Notify start" << std::endl;
        (*_factory) = make_message_factory(&_tree, _descriptor, "", _dynamic_factory);
        //std::cout << "Notify end" << std::endl;
        return true;
    }
};

/// Collect together a set of events, write in one go after running a callback.
class BufferingStreamWriter
{
    BufferingStreamWriter(const BufferingStreamWriter&);
    void operator=(const BufferingStreamWriter&);
public:
    BufferingStreamWriter(shared<a4::io::OutputStream> stream, 
                          MetadataCallback callback,
                          uint32_t buffer_size) 
        : _stream(stream), _callback(callback), _buffer_size(buffer_size)
    {
    }
    
    ~BufferingStreamWriter() { VERBOSE("Destructor flush "); flush(); }
    
    void write(const shared<Message>& m)
    {
        _buffer.push_back(m);
        if (_buffer.size() >= _buffer_size) {
            INFO("Buffer is full!", _buffer.size());
            flush();
        }
    }

    /// Write out Metadata, then events.
    void flush()
    {
        INFO("Flushing...");
        _callback(_stream, _buffer);
        foreach (const shared<Message>& m, _buffer)
            _stream->write(*m);
        _buffer.clear();
    }

private:
    shared<a4::io::OutputStream> _stream;
    std::vector<shared<Message> > _buffer;
    MetadataCallback _callback;
    uint32_t _buffer_size;
};


/// Copies `tree` into the `stream` using information taken from the compiled in
/// Event class.
void copy_chain(TChain& tree, shared<a4::io::OutputStream> stream, 
    MessageFactory* dynamic_factory, const Descriptor* message_descriptor,
    Long64_t entries=-1, const uint32_t metadata_frequency=100000,
    const Long64_t initial_offset=0)
{
    const Long64_t tree_entries = tree.GetEntries();
    if (entries > tree_entries)
        entries = tree_entries;
    if (entries < 0)
        entries = tree_entries;
        
    INFO("Will process ", entries, " entries");
    
    // Nothing to do!
    if (!entries)
        return;
    
    // Disable all branches. Branches get enabled through 
    // TBranch::ResetBit(kDoNotProcess) in the message factory.
    tree.SetBranchStatus("*", false);
    
    // An event_factory is automatically created when the branch pointers change
    // through the Tree Notify() call.
    RootToMessageFactory event_factory;
    
    D3PDMetadataFactory metadata_factory(tree);
    
    auto metadata_callback = bind(&D3PDMetadataFactory::write_metadata, 
                     boost::ref(metadata_factory), _1, _2);
    
    BufferingStreamWriter buffered_stream(stream, metadata_callback, metadata_frequency);
    
    // This is the only place where we say that we're wanting to build the 
    // Event class.
    EventFactoryBuilder builder(tree, message_descriptor, &event_factory, dynamic_factory, 
                                bind(&BufferingStreamWriter::flush, boost::ref(buffered_stream)));
    
    tree.SetNotify(&builder);
    // This line is needed. It seems to sometimes not get called automatically 
    // depending on the underlying TTree.
    builder.Notify();
    
    
    size_t total_bytes_read = 0;
    const Long64_t upper_index = initial_offset+entries;
    
    for (Long64_t i = initial_offset; i < upper_index; i++)
    {
        //std::cout << "Reading event " << i << std::endl;
        size_t read_data = tree.GetEntry(i);
        total_bytes_read += read_data;
                
        if (i % 100 == 0)
            INFO("Progress ", i, " / ", entries, " (", read_data, ")");
        
        shared<Message> event = event_factory();
        
        if (metadata_factory.need_new_metadata(event)) {
            INFO("Need new metadata!");
            buffered_stream.flush();
        }
                
        metadata_factory._input_events += 1;
        metadata_factory._input_bytes_read += read_data;
        
        // Write out one event.
        buffered_stream.write(event);
    }
    
    //Metadata m;
    //m.set_total_events(entries);
    //stream->metadata(m);
    
    INFO("Copied ", entries, " entries (", total_bytes_read, ")");
}
