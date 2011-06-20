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

int a4_main(int argc, char *argv[], ProcessorFactoryPtr pf) 
try
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    po::options_description description("Allowed options");
    description.add_options()
        ("help", "produce help message")
        ("threads", po::value<int>(), "run N multi-threads[-1 for cores]")
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
        ::THREADS = arguments["threads"].as<int>();

    try
    {
        if (::THREADS)
        {
            Inputs inputs(arguments["input"].as<Inputs>());
            fs::path file_path(*inputs.begin());
            boost::shared_ptr<Instructor> instructor(new Instructor(pf, -1 == ::THREADS ? 0 : ::THREADS));
            instructor->processFiles(inputs);
            instructor->results()->print();
        }
        else
        {
            boost::shared_ptr<Processor> processor;
            processor = pf->get_processor();

            string results_file;
            {
                fs::path file_path(argv[1]);
                const string extension(fs::extension(file_path));
            }

            Inputs inputs(arguments["input"].as<Inputs>());
            for(Inputs::const_iterator input = inputs.begin();
                inputs.end() != input;
                ++input)
            {
                fs::path file_path(*input);
                processor->init(file_path);
                processor->processEvents();
            }

            processor->results()->print();
            cout << "Processed events: " << processor->eventsRead() << endl;

        }
    }
    catch(const exception &error)
    {
        cerr << "error: " << error.what() << endl;

        // Clean Up any memory allocated by libprotobuf
        google::protobuf::ShutdownProtobufLibrary();

        return 1;
    }

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
