#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include "a4/application.h"
#include "a4/processor.h"
#include "a4/results.h"

#include "pb/Event.pb.h"

#include "instructor.h"

using namespace std;

namespace fs = boost::filesystem;
namespace po = boost::program_options;

typedef vector<string> Inputs;

int THREADS = 0;

int a4_main(int argc, char *argv[], ProcessingJob &job) 
try
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    po::options_description description("Allowed options");
    description.add_options()
        ("help", "produce help message")
        ("threads", po::value<int>(), "run N multi-threads[0 for # of cores]")
        ("output,o", po::value<string>(), "output file(s)")
        ("results,r", po::value<string>(), "result file(s)")
        ("input,i", po::value<Inputs>(), "input file(s)")
    ;

    po::positional_options_description positional_options;
    positional_options.add("input", -1);

    po::variables_map arguments;
    po::store(po::command_line_parser(argc, argv).
            options(description).positional(positional_options).run(),
            arguments);

    if (2 > argc || arguments.count("help") || !arguments.count("input"))
    {
        cout << "Usage: " << argv[0] << " [Options] input(s)" << endl;
        cout << description << endl;
        return 1;
    }

    if (arguments.count("threads"))
        THREADS = arguments["threads"].as<int>();

    if (arguments.count("output"))
        job.set_output(arguments["output"].as<string>());

    Inputs inputs(arguments["input"].as<Inputs>());

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


    // Clean Up any memory allocated by libprotobuf
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
catch(...)
{
    // Clean Up any memory allocated by libprotobuf
    google::protobuf::ShutdownProtobufLibrary();

    cerr << "Unknown error" << endl;

    return 1;
}
