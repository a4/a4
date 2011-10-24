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

using std::string;
using std::cout; using std::cerr; using std::endl;
using std::ifstream;
using a4::io::A4Input;
using a4::io::A4Output;
using a4::io::InputStream;
using a4::io::OutputStream;

typedef std::vector<string> FileList;

// Write the function ourselves since gcc stl just returns 0 every time
int hardware_concurrency() {
    int num_cores = boost::thread::hardware_concurrency();
    if (num_cores >= 1) return num_cores;
    // Yeah, thought so. Let's get it from cpuinfo since i'm on the train and have no net.
    num_cores = 0;

    // just count the lines starting with "processor "
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while(getline(cpuinfo,line)) {
        std::string subst(line, 0, std::min((size_t)10, line.size()));
        if (subst == "processor " || subst == "processor\t") num_cores++;
    }
    return num_cores;
};

namespace a4{ namespace process{

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
        
SimpleCommandLineDriver::SimpleCommandLineDriver(Configuration* cfg) : configuration(cfg) {
    assert(configuration);
}

void SimpleCommandLineDriver::simple_thread(SimpleCommandLineDriver* self, 
    Processor* p, int limit, ProcessStats& stats) {
    // This is MY processor! (makes sure processor is deleted on function exit)
    // The argument to this function should be a move into a unique...
    unique<Processor> processor(p);
    
    // It is safe to get these, even if they are not used.
    // The ownership of these is shared with A4Input/Output.
    shared<OutputStream> outstream, resstream;
    
    
    shared<ObjectBackStore> bs(new ObjectBackStore());
    if (self->out) {
        shared<OutputStream> outstream = self->out->get_stream();
        self->set_outstream(p, outstream);
        outstream->set_compression("ZLIB", 1);
    }
    if (self->res) {
        resstream = self->res->get_stream();
        resstream->set_compression("ZLIB", 9);
    }
    self->set_backstore(p, bs);
    self->set_store_prefix(p);
    
    boost::chrono::thread_clock::time_point start = boost::chrono::thread_clock::now();

    // Try as long as there are inputs
    A4Message current_metadata;
    int cnt = 0;
    while (shared<InputStream> instream = self->in->get_stream()) {
        self->set_instream(p, instream);
        while (A4Message msg = instream->next()) {
            if (instream->new_metadata()) {
                A4Message new_metadata = instream->current_metadata();
                if (current_metadata) {
                    // Check if we merge the old into the new metadata
                    // and wait with writing it.
                    bool merge = false;

                    // Determine if merging is necessary
                    if (self->metakey != "") {
                        google::protobuf::Message & m = *current_metadata.message;
                        const google::protobuf::FieldDescriptor* fd = m.GetDescriptor()->FindFieldByName(self->metakey);
                        if (!fd) {
                            const std::string & classname = m.GetDescriptor()->full_name();
                            throw a4::Fatal(classname, " has no member ", self->metakey, " necessary for metadata merging!");
                        }
                        if (fd->is_repeated() && (m.GetReflection()->FieldSize(m, fd)) > 1) {
                            throw a4::Fatal(fd->full_name(), " has already multiple ", self->metakey, " entries - cannot achieve desired granularity!");
                        }
                        std::string s1 = current_metadata.field_as_string(self->metakey);
                        std::string s2 = new_metadata.field_as_string(self->metakey);
                        if (s1 == s2) merge = true;
                    }

                    if (merge) {
                        std::cerr<< "Merging " << current_metadata.message->ShortDebugString() << "\n and " << new_metadata.message->ShortDebugString() << std::endl;
                        current_metadata = current_metadata + new_metadata;
                        std::cerr << "to "<<current_metadata.message->ShortDebugString() << std::endl;

                    } else {
                        if (self->out) p->metadata(*current_metadata.message);
                        if (self->res) {
                            bs->to_stream(*resstream);
                            resstream->metadata(*current_metadata.message);
                            bs.reset(new ObjectBackStore()); 
                            self->set_backstore(p, bs);
                            self->set_store_prefix(p);
                        }
                        current_metadata = new_metadata;
                    }
                } else current_metadata = new_metadata;

                self->set_metadata(p, current_metadata);
                p->process_new_metadata();
            }
            p->process_message(msg);
            
            if (++cnt == limit) {
                // Stream store to output
                if (self->res) bs->to_stream(*resstream);
                stats.cputime = boost::chrono::thread_clock::now() - start;
                stats.bytes += instream->ByteCount();
                stats.events = cnt;
                return;
            }
        }
        
        // We're about to get a new stream, record how many this one had
        stats.bytes += instream->ByteCount();
        
        if (instream->error()) {
            std::cerr << "stream error in thread " << boost::this_thread::get_id() << std::endl;
            return;
        }
    }
    // Stream store to output
    if (self->res) bs->to_stream(*resstream);
    
    stats.cputime = boost::chrono::thread_clock::now() - start;
    stats.events = cnt;
}

Processor * SimpleCommandLineDriver::new_initialized_processor() {
    Processor * p = configuration->new_processor();
    configuration->setup_processor(*p);
    return p;
}

int SimpleCommandLineDriver::main(int argc, const char * argv[]) 
try
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against
    //GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    
    int n_threads, number;
    int hw_threads = hardware_concurrency();
    FileList inputs;
    po::options_description commandline_options;
    po::options_description config_file_options;
    string config_filename = string(argv[0]) + ".ini";
    string output = "", results = "";
    metakey = "";

    // Define all 
    po::options_description popt("Processing options");
    popt.add_options()
        ("help", "print this help message")
        ("input,i", po::value<FileList>(&inputs), "input file(s)")
        ("output,o", po::value<string>(&output), "output file")
        ("results,r", po::value<string>(&results), "result file")
        ("number,n", po::value<int>(&number)->default_value(-1), "maximum number of events to process (default: all)")
        ("per,p", po::value<string>(&metakey), "granularity of output by metadata key (e.g. period, run, lumiblock...). Default is input granularity.")
        ("config,c", po::value<string>(), (string("configuration file [default is '") + config_filename + "']").c_str());

    po::positional_options_description positional_options;
    positional_options.add("input", -1);

    po::options_description cfgopt("Configuration: (section [config] in configuration file)");
    cfgopt.add_options()
        ("config.threads,t", po::value<int>(&n_threads)->default_value(hw_threads), "run N multi-threads [# of cores]");

    po::options_description useropt = configuration->get_options();

    commandline_options.add(popt);
    commandline_options.add(cfgopt);
    commandline_options.add(useropt);
    config_file_options.add(cfgopt);
    config_file_options.add(useropt);

    // Parse command line first
    std::vector<string> _argvs;
    for (int i = 1; i < argc; i++) _argvs.push_back(string(argv[i]));

    po::variables_map arguments;
    po::store(po::command_line_parser(_argvs).options(commandline_options).positional(positional_options).run(), arguments);

    if (2 > argc || arguments.count("help") || !arguments.count("input"))
    {
        cout << "Usage: " << argv[0] << " [Options] input file(s)" << endl;
        cout << commandline_options << endl;
        return 1;
    }

    // Parse config file
    bool explicit_config_file = false;
    if (arguments.count("config")) {
        config_filename = arguments["config"].as<string>();
        explicit_config_file = true;        
    };
    std::ifstream config_file(config_filename.c_str());
    if (!config_file && explicit_config_file) {
        throw std::runtime_error("Configuration file '" + config_filename + "' not found!");
    } else if (config_file && !explicit_config_file) {
        cout << "Using implicit config file '" + config_filename + "'. Override this with -c 'other_configfile.ini'." << endl;
    }
    po::store(po::parse_config_file(config_file, config_file_options), arguments);

    // After finishing all option reading, notify the result
    po::notify(arguments);
    configuration->read_arguments(arguments);

    // DEBUG
    foreach(string & i, inputs) { cout << "inputs += " << i << endl; } 
    if (number != -1) n_threads = 1;
    cout << "output = " << output << endl;
    cout << "results = " << results << endl;
    cout << "config_filename = " << config_filename << endl;
    cout << "n_threads = " << n_threads << endl;

    // Set up I/O
    in.reset(new A4Input("A4 Input Files"));
    foreach(string & i, inputs) in->add_file(i);

    if (output.size()) out.reset(new A4Output(output, "A4 Output File"));

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
            Processor * p = new_initialized_processor();
            //threads.push_back(boost::thread(std::bind(&simple_thread, this, processors[i])));
            threads.push_back(boost::thread(std::bind(&simple_thread, this, p, -1, boost::ref(stats[i]))));
        };
        foreach(boost::thread & t, threads) t.join();
    } else {
        Processor * p = new_initialized_processor();
        simple_thread(this, p, number, stats[0]);
    }
    
    ProcessStats total;
    foreach(const ProcessStats& s, stats) 
        total += s;
    
    chrono::duration<double> walltime = chrono::steady_clock::now() - start;
    
    std::cout << "A4 processed " << total.events << " objects "
              << "in " << walltime.count() << " seconds. (" << (total.events / walltime.count()) << "Hz)" << std::endl;

    std::cout << "CPU time: " << total.cputime << " (" << (total.events / total.cputime.count()) << "Hz)" << std::endl;
    
    // Clean Up any memory allocated by libprotobuf
    //google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
catch(std::exception &x)
{
    // Clean Up any memory allocated by libprotobuf
    //google::protobuf::ShutdownProtobufLibrary();

    cerr << "Exception: " << x.what() << endl;
    return 1;
}

};}; // namespace a4::process
