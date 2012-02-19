#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include <boost/chrono.hpp>
namespace chrono = boost::chrono;
typedef chrono::duration<double> duration;

#include <boost/thread.hpp>

#include <a4/application.h>
#include <a4/input.h>
#include <a4/output.h>
#include <a4/cpu_info.h>

using std::string;
using std::ifstream;
using a4::io::A4Input;
using a4::io::A4Output;
using a4::io::InputStream;
using a4::io::OutputStream;

typedef std::vector<string> FileList;

namespace a4 { 
namespace process {

class ProcessStats {
public:
    size_t events, bytes;
    duration cputime;
    
    ProcessStats() : events(0), bytes(0), cputime(0) {}
    
    ProcessStats& operator+=(const ProcessStats& rhs) {
        events += rhs.events;
        bytes += rhs.bytes;
        cputime += rhs.cputime;
        return *this;
    }
};
        
SimpleCommandLineDriver::SimpleCommandLineDriver(Configuration* cfg) 
    : configuration(cfg) 
{
    assert(configuration);
}

class BaseOutputAdaptor : public OutputAdaptor {
    public:
        shared<A4Message> current_metadata;
        std::string merge_key, split_key; 
        A4Output* out;
        A4Output* res;
        shared<ObjectBackStore> backstore;

        BaseOutputAdaptor(Driver* d, Processor* p, bool forward_metadata, 
                          A4Output* out, A4Output* res) 
            : out(out), res(res), forward_metadata(forward_metadata), 
              in_block(false), driver(d), p(p), last_postfix("") 
        {
            merge_key = split_key = "";
            outstream.reset();
            resstream.reset();
            backstore.reset();
            start_block(); // This writes no metadata
        }

        void start_block(std::string postfix="") {
            if (out and (!outstream or postfix != last_postfix)) {
                outstream = out->get_stream(postfix);
                outstream->set_compression("ZLIB", 1);
                if (forward_metadata) 
                    outstream->set_forward_metadata();
            }
            if (res and (!resstream or postfix != last_postfix)) {
                resstream = res->get_stream(postfix);
                resstream->set_compression("ZLIB", 1);
                resstream->set_forward_metadata();
            }
            backstore.reset(new ObjectBackStore());
            driver->set_store(p, backstore->store());
            if (outstream and forward_metadata and current_metadata) {
                current_metadata->unionize();                
                outstream->metadata(*current_metadata->message());
            }
            in_block = true;
        }

        void end_block() {            
            if (resstream && current_metadata) {
                current_metadata->unionize();
                resstream->metadata(*current_metadata->message());
            }
            if (backstore && resstream) {
                backstore->to_stream(*resstream);
            }
            backstore.reset();
            if (outstream and !forward_metadata and current_metadata) {
                current_metadata->unionize();
                outstream->metadata(*current_metadata->message());
            }
            in_block = false;
        }

        void new_outgoing_metadata(shared<const A4Message> new_metadata) {
            // Check if we merge the old into the new metadata
            // and hold off on writing it.
            bool merge = false;
            shared<A4Message> old_metadata = current_metadata;

            // Determine if merging is necessary
            if (new_metadata && old_metadata && merge_key != "") {
                std::string s1 = old_metadata->assert_field_is_single_value(merge_key);
                std::string s2 = new_metadata->assert_field_is_single_value(merge_key);
                if (s1 == s2) merge = true;
            }

            if (merge) {
                //std::cerr << "Merging\n" << old_metadata.message()->ShortDebugString()
                //          << "\n...and...\n" << new_metadata.message()->ShortDebugString() << std::endl;
                *current_metadata += *new_metadata;
                //std::cerr << "...to...\n" << current_metadata->message()->ShortDebugString() << std::endl;
            } else { // Normal action in case of new metadata
                // If we are in charge of metadata, start a new block now...
                end_block();

                std::string postfix = "";
                if (new_metadata && split_key != "") 
                    postfix = new_metadata->assert_field_is_single_value(split_key);
                current_metadata.reset(new A4Message(*new_metadata));

                start_block(postfix);
            } // end of normal action in case of new metadata

        }

        virtual void metadata(shared<const A4Message> m) {
            FATAL("To write metadata manually, you have to change the metadata_behavior of the Processor!");
        }
    
        void write(shared<const A4Message> m) {
            if (!in_block) 
                FATAL("Whoa?? Writing outside of a metadata block? How did you do this?");
            
            if (outstream) 
                outstream->write(m);
        }

    protected:
        bool forward_metadata;
        shared<OutputStream> outstream, resstream;

        bool in_block;
        Driver* driver;
        Processor* p;
        std::string last_postfix;
};

class ManualOutputAdaptor : public BaseOutputAdaptor {
    public:
        ManualOutputAdaptor(Driver* d, Processor* p, bool forward_metadata, A4Output* out, A4Output* res) 
            : BaseOutputAdaptor(d, p, forward_metadata, out, res) {}
        
        void metadata(shared<const A4Message> m) {
            new_outgoing_metadata(m);
        }
};


void SimpleCommandLineDriver::simple_thread(SimpleCommandLineDriver* self, 
    Processor* p, int limit, ProcessStats& stats) {
    // This is MY processor! (makes sure processor is deleted on function exit)
    // The argument to this function should be a move into a unique...
    unique<Processor> processor(p);
    unique<BaseOutputAdaptor> output_adaptor;
    
    bool metadata_forward;
    bool auto_metadata = false;
    switch(p->get_metadata_behavior()) {
        case Processor::AUTO:
            metadata_forward = (self->metakey == ""); // forward if no merging
            auto_metadata = true;
            output_adaptor.reset(new BaseOutputAdaptor(self, p, metadata_forward, self->out.get(), self->res.get()));
            break;
        case Processor::MANUAL_FORWARD:
            if (self->metakey != "") FATAL("This program is not compatible with metadata merging!"); // forward if no merging
            // fall through to ...
        case Processor::DROP:
            metadata_forward = true;
            output_adaptor.reset(new ManualOutputAdaptor(self, p, metadata_forward, self->out.get(), self->res.get()));
            break;
        case Processor::MANUAL_BACKWARD:
            metadata_forward = false;
            output_adaptor.reset(new ManualOutputAdaptor(self, p, metadata_forward, self->out.get(), self->res.get()));
            break;
        default:
            FATAL("Unknown metadata behaviour specified: ", p->get_metadata_behavior());
    }
    output_adaptor->merge_key = self->metakey;
    output_adaptor->split_key = self->split_metakey;
    
    boost::chrono::thread_clock::time_point start = boost::chrono::thread_clock::now();

    self->set_output_adaptor(p, output_adaptor.get());

    // Try as long as there are inputs
    int cnt = 0;
    bool run = true;
    while (shared<InputStream> instream = self->in->get_stream()) {
        if (!run)
            break;
        
        if (instream->new_metadata()) { // Start of new metadata block
            shared<const A4Message> c_new_metadata = instream->current_metadata();

            // WARNING: "no metadata" events are subsumed into previous/next metadata here!
            if (c_new_metadata) {
                // Process end of old metadata block, if any.
                if (output_adaptor->current_metadata)
                    p->process_end_metadata();

                // Process start of new incoming metadata block (this may modify new_metadata)
                // In Manual Mode, this may also trigger a callback in the output_adaptor.
                // Note that set_metadata is only called here, since the processor should only
                // ever see incoming metadata (by contract).

                self->set_metadata(p, c_new_metadata);
                shared<A4Message> new_metadata = p->process_new_metadata();
                if (auto_metadata){
                    if (new_metadata) {
                        output_adaptor->new_outgoing_metadata(new_metadata);
                    } else {
                        output_adaptor->new_outgoing_metadata(c_new_metadata);
                    }
                } else if (new_metadata) {
                    FATAL("You must not modify metadata in process_new_metadata if auto_metadata is off!");
                }

            }
        }

        while (shared<A4Message> msg = instream->next_with_metadata()) {
            if (!run)
                break;
            
            if (instream->new_metadata()) { // Start of new metadata block
                shared<const A4Message> c_new_metadata = instream->current_metadata();

                // WARNING: "no metadata" events are subsumed into previous/next metadata here!
                if (c_new_metadata) {
                    // Process end of old metadata block, if any.
                    if (output_adaptor->current_metadata)
                        p->process_end_metadata();

                    // Process start of new incoming metadata block (this may modify new_metadata)
                    // In Manual Mode, this may also trigger a callback in the output_adaptor.
                    // Note that set_metadata is only called here, since the processor should only
                    // ever see incoming metadata (by contract).
                    self->set_metadata(p, c_new_metadata);
                    shared<A4Message> new_metadata = p->process_new_metadata();
                    if (auto_metadata){
                        if (new_metadata) {
                            output_adaptor->new_outgoing_metadata(new_metadata);
                        } else {
                            output_adaptor->new_outgoing_metadata(c_new_metadata);
                        }
                    } else if (new_metadata) {
                        FATAL("You must not modify metadata in process_new_metadata if auto_metadata is off!");
                    }
                }
            }
            // Do not send metadata messages to process()
            if (msg->metadata())
                continue;

            self->set_store(p, output_adaptor->backstore->store());
            try {
                process_rerun_systematics(p, msg);
            } catch (...) {
                ERROR("Caught an exception in the processor");
                if (msg) {
                    try {
                        auto protomsg = msg->message();
                        ERROR(protomsg->GetDescriptor()->full_name(), ":");
                        ERROR(protomsg->DebugString());
                    } catch (...) {
                        ERROR("Could not show event");
                    }
                } else {
                    ERROR("Processed message is invalid");
                }
                throw;
            }

            // Skip if the user wants us to.
            if (p->skip_to_next_metadata) {
                instream->skip_to_next_metadata();
                p->skip_to_next_metadata = false;
            }

            // Check if we reached limit
            if (++cnt == limit)
                run = false;
        }
        
        // We're about to get a new stream, record how many this one had
        stats.bytes += instream->ByteCount();
        
        if (instream->error()) {
            ERROR("stream error in thread ", boost::this_thread::get_id());
            return;
        }
    }

    // Stream store to output
    output_adaptor->end_block();
    stats.cputime = boost::chrono::thread_clock::now() - start;
    stats.events = cnt;
}

Processor* SimpleCommandLineDriver::new_initialized_processor() {
    Processor* p = configuration->new_processor();
    configuration->setup_processor(*p);
    return p;
}

int SimpleCommandLineDriver::main(int argc, const char* argv[]) 
try
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against
    //GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    
    int n_threads, number;
    int hw_threads = get_cpuinfo().physical_cores;
    bool no_gdb = false;
    FileList inputs;
    po::options_description commandline_options;
    po::options_description config_file_options;
    string config_filename = string(argv[0]) + ".ini";
    string output = "", results = "";
    metakey = ""; split_metakey = "";

    // Define all 
    po::options_description popt("Processing options");
    popt.add_options()
        ("help", "print this help message")
        ("disable-gdb", po::bool_switch(&no_gdb), "switch of internal segfault handling")
        ("input,i", po::value<FileList>(&inputs), "input file(s)")
        ("output,o", po::value<string>(&output), "output file")
        ("results,r", po::value<string>(&results), "result file")
        ("number,n", po::value<int>(&number)->default_value(-1), "maximum number of events to process (default: all)")
        ("per,p", po::value<string>(&metakey), "granularity of output by metadata key (e.g. period, run, lumiblock...). Default is input granularity.")
        ("split-per,s", po::value<string>(&split_metakey), "granularity of output by metadata key (e.g. period, run, lumiblock...). Default is input granularity.")
        ("config,c", po::value<string>(), (string("configuration file [default is '") + config_filename + "']").c_str());

    po::positional_options_description positional_options;
    positional_options.add("input", -1);

    po::options_description cfgopt("Configuration: (section [config] in configuration file)");
    cfgopt.add_options()
        ("config.threads,t", po::value<int>(&n_threads)->default_value(hw_threads), "run N multi-threads [# of cores]");

    po::options_description useropt;
    configuration->add_options(useropt.add_options());

    commandline_options.add(popt);
    commandline_options.add(cfgopt);
    commandline_options.add(useropt);
    config_file_options.add(cfgopt);
    config_file_options.add(useropt);

    // Parse command line first
    std::vector<string> _argvs;
    for (int i = 1; i < argc; i++)
        _argvs.push_back(string(argv[i]));

    po::variables_map arguments;
    po::store(po::command_line_parser(_argvs)
        .options(commandline_options)
        .positional(positional_options).run(), arguments);

    if (2 > argc || arguments.count("help") || !arguments.count("input"))
    {
        std::cout << "Usage: " << argv[0] << " [Options] input file(s)" << std::endl;
        std::cout << commandline_options << std::endl;
        return 1;
    }

    // Parse config file
    bool explicit_config_file = false;
    if (arguments.count("config")) {
        config_filename = arguments["config"].as<string>();
        explicit_config_file = true;        
    }
    
    std::ifstream config_file(config_filename.c_str());
    if (!config_file && explicit_config_file) {
        throw std::runtime_error("Configuration file '" + config_filename + "' not found!");
    } else if (config_file && !explicit_config_file) {
        std::cout << "Using implicit config file '" << config_filename 
                  << "'. Override this with -c 'other_configfile.ini'." << std::endl;
    }
    po::store(po::parse_config_file(config_file, config_file_options), arguments);

    // After finishing all option reading, notify the result
    po::notify(arguments);
    configuration->read_arguments(arguments);

    if (not no_gdb) {
        a4::Fatal::enable_throw_on_segfault();
    }

    if (number != -1) n_threads = 1;

    // DEBUG
    //foreach (string& i, inputs) { cout << "inputs += " << i << endl; } 
    //cout << "output = " << output << endl;
    //cout << "results = " << results << endl;
    //cout << "config_filename = " << config_filename << endl;
    //cout << "n_threads = " << n_threads << endl;

    // Set up I/O
    in.reset(new A4Input("A4 Input Files"));
    foreach(string& i, inputs) 
        in->add_file(i);

    if (output.size()) 
        out.reset(new A4Output(output, "A4 Output File"));

    shared<A4Output> a4results;
    if (results.size()) {
        if (results == output) {
            res = out;
        } else {
            res.reset(new A4Output(results, "A4 Results File"));
        }
    }

    std::vector<ProcessStats> stats(n_threads);
    if (n_threads > 1) {
        std::vector<boost::thread> threads;
        for (int i = 0; i < n_threads; i++) {
            Processor* p = new_initialized_processor();
            //threads.push_back(boost::thread(std::bind(&simple_thread, this, processors[i])));
            threads.push_back(boost::thread(std::bind(&simple_thread, this, p, -1, boost::ref(stats[i]))));
        };
        foreach(boost::thread& t, threads) t.join();
    } else {
        Processor* p = new_initialized_processor();
        simple_thread(this, p, number, stats[0]);
    }
    
    ProcessStats total;
    foreach(const ProcessStats& s, stats) 
        total += s;
    
    chrono::duration<double> walltime = chrono::steady_clock::now() - start;
    
    VERBOSE("A4 processed ", total.events, " objects in ", walltime.count(),
            " seconds. (", total.events / walltime.count(), "Hz)");

    VERBOSE("CPU time: ", total.cputime, " (", total.events / total.cputime.count(), "Hz)");
    
    const double megabytes = total.bytes / (1024.*1024.);
    VERBOSE("Total data read ", megabytes, " (MB) Rate: ", megabytes / walltime.count(), " (MB/s)");
    
    // Clean Up any memory allocated by libprotobuf
    //google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
catch(std::exception& x)
{
    // Clean Up any memory allocated by libprotobuf
    //google::protobuf::ShutdownProtobufLibrary();

    std::cerr << "Exception: " << x.what() << std::endl;
    return 1;
}

};}; // namespace a4::process
