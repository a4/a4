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

#include "common.h"
#include "period_mapping.h"

//#include <a4/atlas/ntup/photon/Event.pb.h>
//#include <a4/atlas/ntup/smwz/Event.pb.h>

#include <a4/atlas/Event.pb.h>
#include <a4/atlas/EventMetaData.pb.h>

using a4::atlas::EventMetaData;
using a4::atlas::ProcessingStep;
using a4::atlas::InputFile;

#include <a4/root/test/Event.pb.h>

typedef std::vector<shared<Message> > MessageBuffer;
typedef boost::function<void (shared<a4::io::OutputStream>, const MessageBuffer&)> MetadataCallback;
typedef boost::function<void ()> FlushCallback;



class MetadataFactory
{
    
    chrono::steady_clock::time_point _start_wallclock;
    chrono::thread_clock::time_point _start_cpuclock;
    
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
            std::cout << "Couldn't get GRL string.." << std::endl;
        }
    }
    
    void compute_info(const MessageBuffer& buffer)
    {   
        super::compute_info(buffer);
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
        std::cout << "Processing new file: " << _tree.GetCurrentFile()->GetName() << std::endl;
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
    
    ~BufferingStreamWriter() { std::cout << "Destructor flush "<< std::endl; flush(); }
    
    void write(const shared<Message>& m)
    {
        _buffer.push_back(m);
        if (_buffer.size() >= _buffer_size) {
            std::cout << "Buffer is full!" << _buffer.size() << std::endl;
            flush();
        }
    }

    /// Write out Metadata, then events.
    void flush()
    {
        std::cout << "Flushing.." << std::endl;
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
    
    D3PDMetadataFactory metadata_factory(tree);
    
    BufferingStreamWriter buffered_stream(
        stream, bind(&D3PDMetadataFactory::write_metadata, 
                     boost::ref(metadata_factory), _1, _2), metadata_frequency);
    
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
            std::cout << "Progress " << i << " / " << entries << " (" << read_data << ")" << std::endl;
        
        shared<Message> event = event_factory();
        
        if (metadata_factory.need_new_metadata(event)) {
            std::cout << "Need new metadata!" << std::endl;
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
    
    std::cout << "Copied " << entries << " entries (" << total_bytes_read << ")" << std::endl;
}

bool string_endswith(const std::string& s, const std::string& end) 
{
    auto pos = s.rfind(end);
    if (pos == std::string::npos) return false;
    return s.substr(pos) == end;
}

int main(int argc, char ** argv) {
    a4::Fatal::enable_throw_on_segfault();
        
    assert(gROOT->GetVersionCode() == ROOT_VERSION_CODE && "Check build ROOT version check against dynamic library");
    
    chrono::steady_clock::time_point wallstart = chrono::steady_clock::now();

    namespace po = boost::program_options;

    std::string tree_name, tree_type, output_file, compression_type;
    std::vector<std::string> input_files;
    Long64_t event_count = -1, initial_offset = 0;
    uint32_t metadata_frequency = 100000;
    
    #ifdef HAVE_SNAPPY
    const char* default_compression = "SNAPPY";
    #else
    const char* default_compression = "ZLIB 9";
    #endif

    po::positional_options_description p;
    p.add("input", -1);

    po::options_description commandline_options("Allowed options");
    commandline_options.add_options()
        ("help,h", "produce help message")
        ("tree-name,t", po::value<std::string>(&tree_name), "input TTree name")
        ("tree-type,T", po::value<std::string>(&tree_type)->default_value("test"),
            "which event factory to use (SMWZ, PHOTON, test or .proto file with Event message)")
        ("input,i", po::value<std::vector<std::string> >(&input_files), "input file names")
        ("output,o", po::value<std::string>(&output_file)->default_value("test_io.a4"), "output file name")
        ("number,n", po::value<Long64_t>(&event_count), "number of events to process (-1=all available)")
        ("offset,O", po::value<Long64_t>(&initial_offset), "initial offset (0=first event)")
        ("compression-type,C", po::value(&compression_type)->default_value(default_compression), "compression level '[TYPE] [LEVEL]'")
        ("metadata-frequency,m", po::value(&metadata_frequency)->default_value(metadata_frequency), "Metadata frequency [N] (1 per N)")
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
    
    std::cout << "Adding input files" << std::endl;
    foreach (const std::string& input_file, input_files) {
        if (string_endswith(input_file, ".txt")) {
        //if (input_file.substr(input_file.rfind(".txt")) == ".txt") {
            ifstream input_file_stream(input_file);

            std::string argStr;
            std::ifstream ifs(input_file);
            std::getline(ifs, argStr);

            // split by ','
            std::vector<std::string> fileList;
            for (size_t i = 0, n; i <= argStr.length(); i = n+1)
            {
                n = argStr.find_first_of(',',i);
                if (n == std::string::npos)
                    n = argStr.length();
                std::string tmp = argStr.substr(i,n-i);
                fileList.push_back(tmp);
            }
            foreach (const std::string& x, fileList) {
                std::cout << "--Adding sub input file: '" << x << "'" << std::endl;
                input.Add(x.c_str());
            }
        } else
        {
            std::cout << "--Adding input '" << input_file << "'" << std::endl;
            input.Add(input_file.c_str());
        }
    }
    
    

    a4::io::A4Output a4o(output_file, "Event");
    shared<a4::io::OutputStream> stream = a4o.get_stream("", true);
    std::stringstream ss(compression_type);
    std::string ctype; int level;
    ss >> ctype >> level;
    stream->set_compression(ctype, level);
    
    class ErrorCollector : public MultiFileErrorCollector {
        void AddError(const std::string& filename, int line, int column, const std::string& message) {
            FATAL("Proto import error in ", filename, ":", line, ":", column, " ", message);
        }
    };
    
    DiskSourceTree source_tree;
    source_tree.MapPath("", current_path().string());
    source_tree.MapPath("a4/root/", "");
    
    SourceTreeDescriptorDatabase source_tree_db(&source_tree);
    ErrorCollector e;
    source_tree_db.RecordErrorsTo(&e);
    DescriptorPool source_pool(&source_tree_db, source_tree_db.GetValidationErrorCollector());
    
    DescriptorPoolDatabase builtin_pool(*DescriptorPool::generated_pool());
    MergedDescriptorDatabase merged_pool(&builtin_pool, &source_tree_db);
    
    DescriptorPool pool(&merged_pool, source_tree_db.GetValidationErrorCollector());
    
    DynamicMessageFactory dynamic_factory(&pool);
    dynamic_factory.SetDelegateToGeneratedFactory(true);
    
    const Descriptor* descriptor = NULL;
    /*
    if (tree_type == "test")
        descriptor = a4::root::test::Event::descriptor();
    else if (tree_type == "PHOTON")
        descriptor = a4::atlas::ntup::photon::Event::descriptor();
    else if (tree_type == "SMWZ")
        descriptor = a4::atlas::ntup::smwz::Event::descriptor();
        
    else
    */
    {        
        const FileDescriptor* file_descriptor = pool.FindFileByName(tree_type);
                
        descriptor = file_descriptor->FindMessageTypeByName("Event");
        
        if (!descriptor)
            FATAL("Couldn't find an \"Event\" class in ", tree_type);
    }
    
    {
        chrono::thread_clock::time_point cpustart = chrono::thread_clock::now();
        
        copy_chain(input, stream, &dynamic_factory, descriptor, event_count, 
            metadata_frequency, initial_offset);
            
            
        duration cputime  = chrono::thread_clock::now() - cpustart,
                 walltime = chrono::steady_clock::now() - wallstart;
        std::cout << "Took " << walltime << " to run (" 
                  << cputime << " CPU)" << std::endl;
    }

    foreach(std::string s, get_list_of_leaves()) {
        std::cout << "-" << s << std::endl;
    }
}


