#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <exception>

#include "a4/application.h"

//#include "instructor.h"

using namespace std;

namespace po = boost::program_options;

typedef vector<string> FileList;

int a4_main(int argc, char *argv[])
try
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against
    //GOOGLE_PROTOBUF_VERIFY_VERSION;
    string config_filename = string(argv[0]) + ".ini";

    po::options_description commandline_options;
    po::options_description config_file_options;

    po::options_description popt("Processing options");
    popt.add_options()
        ("help", "produce help message")
        ("input,i", po::value<FileList>(), "input file(s)")
        ("output,o", po::value<string>(), "output file")
        ("results,r", po::value<string>(), "result file")
        ("config,c", po::value<string>(), (string("configuration file [") + config_filename + "]").c_str());

    po::positional_options_description positional_options;
    positional_options.add("input", -1);

    po::options_description cfgopt("Configuration:");
    cfgopt.add_options()
        ("threads", po::value<int>(), "run N multi-threads [# of cores]");

    commandline_options.add(popt);
    commandline_options.add(cfgopt);

    config_file_options.add(cfgopt);

    // Parse command line first
    po::variables_map arguments;
    po::store(po::command_line_parser(argc, argv).options(commandline_options).positional(positional_options).run(), arguments);

    if (2 > argc || arguments.count("help") || !arguments.count("input"))
    {
        cout << "Usage: " << argv[0] << " [Options] input(s)" << endl;
        cout << commandline_options << endl;
        return 1;
    }

    // Parse config file
    if (arguments.count("config")) config_filename = arguments["config"].as<string>();
    ifstream config_file(config_filename);
    po::store(po::parse_config_file(config_file, config_file_options), arguments);

    int threads = 0;
    if (arguments.count("threads"))
        threads = arguments["threads"].as<int>();

    std::cout << "Threads = " << threads << std::endl;
/*
    if (arguments.count("output"))
        job.set_output(arguments["output"].as<string>());

    FileList inputs(arguments["input"].as<FileList>());

    try
    {
        job.process_work_units(inputs, THREADS);
        job.finalize();
    }
    catch(const exception &error)
    {
        cerr << "error: " << error.what() << endl;

        // Clean Up any memory allocated by libprotobuf
        google::protobuf::ShutdownProtobufLibrary();

        return 1;
    }

    if (arguments.count("results"))
        job.results()->to_file(arguments["results"].as<string>());

*/
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
