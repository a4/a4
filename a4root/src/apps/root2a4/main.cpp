#include <iostream>
#include <fstream>
using std::ifstream;
#include <iterator>
using std::istream_iterator;

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

#include <a4/types.h>
#include <a4/output.h>
#include <a4/output_stream.h>
#include <a4/input.h>

#include <a4/root/copy_chain.h>
#include <a4/root/common.h>

bool string_endswith(const std::string& s, const std::string& end) 
{
    auto pos = s.rfind(end);
    if (pos == std::string::npos) return false;
    return s.substr(pos) == end;
}

#include <TROOT.h>
#include <RVersion.h>

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
    
    INFO("Adding input files");
    foreach (const std::string& input_file, input_files) {
        if (string_endswith(input_file, ".txt")) {
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
                INFO("--Adding sub input file: '", x, "'");
                input.Add(x.c_str());
            }
        } else
        {
            INFO("--Adding input '", input_file, "'");
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
#ifdef BOOST_CHRONO_HAS_THREAD_CLOCK
        chrono::thread_clock::time_point cpustart = chrono::thread_clock::now();
#endif
        
        copy_chain(input, stream, &dynamic_factory, descriptor, event_count, 
            metadata_frequency, initial_offset);
            
#ifdef BOOST_CHRONO_HAS_THREAD_CLOCK
        duration cputime  = chrono::thread_clock::now() - cpustart;
#endif
        duration walltime = chrono::steady_clock::now() - wallstart;
        INFO("Took ", walltime, " to run"
#ifdef BOOST_CHRONO_HAS_THREAD_CLOCK
             "(", cputime, " CPU)"
#endif
            );
    }

    foreach(std::string s, get_list_of_leaves()) {
        std::cout << "-" << s << std::endl;
    }
}


