#include <algorithm>

#include <limits>
using std::numeric_limits;

#include <utility>
#include <string>
#include <vector>

#include <iostream>
#include <iomanip>

#include <map>
#include <unordered_map>

#include <boost/program_options.hpp>

#include <google/protobuf/text_format.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
using google::protobuf::DescriptorPool;
using google::protobuf::FileDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

#include <a4/input.h>
#include <a4/message.h>


void dump_message(const Message& message, const std::vector<std::string>& vars) {
    if (vars.size()) {
        // specialized code
        throw a4::Fatal("Not implemented yet");
    } else {
        std::string str;
        google::protobuf::TextFormat::PrintToString(message, &str);
        std::cout << str << std::endl;
    }    
}

int main(int argc, char ** argv) {
    a4::Fatal::enable_throw_on_segfault();

    namespace po = boost::program_options;

    std::vector<std::string> input_files, variables;
    size_t event_count = -1, event_index = -1;
    bool collect_stats = false;
    
    po::positional_options_description p;
    p.add("input", -1);

    po::options_description commandline_options("Allowed options");
    commandline_options.add_options()
        ("help,h", "produce help message")
        ("event-index,i", po::value(&event_index)->default_value(0), "event to start dumping from (starts at 0)")
        ("count,c", po::value(&event_count)->default_value(1), "number to dump'")
        ("input", po::value(&input_files), "input file names (runs once per specified file)")
        ("var,v", po::value(&variables), "variables to dump (defaults to all)")
        ("collect-stats,S", po::value(&collect_stats), "should collect statistics for all numeric variables")
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
    
    a4::io::A4Input in;
    foreach (std::string filename, input_files)
        in.add_file(filename);
        
    shared<a4::io::InputStream> stream = in.get_stream();
    
    const std::vector<std::vector<a4::io::A4Message>>& all_metadata = stream->all_metadata();
    
    std::cout << "Got " << all_metadata.size() << " header(s)" << std::endl;
    
    int i = 0;
    foreach (const auto& header, all_metadata) {
        std::cout << "Header " << i++ << std::endl;
        foreach (const a4::io::A4Message& metadata, header) {
            dump_message(*metadata.message, variables);
        }   
    }
}

