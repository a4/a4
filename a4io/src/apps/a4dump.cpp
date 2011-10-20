#include <limits>
using std::numeric_limits;
#include <utility>
#include <string>
#include <iostream>
#include <map>

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

class StatsCollector {
    class ColumnSizeMeasurer {
    public:
        std::vector<int> lengths;
        size_t current_index;
        
        
        template<typename Streamable>
        friend ColumnSizeMeasurer& operator<< (ColumnSizeMeasurer& o, Streamable const& s) {
            std::stringstream stream;
            stream << s;
            const int this_length = stream.tellp();
            if (o.current_index > o.lengths.size())
                o.lengths.push_back(this_length);
            else {
                if (o.lengths[o.current_index] < this_length)
                    o.lengths[o.current_index] = this_length;
            }
            o.current_index++;
            return o;
        }
        
        void newline() {
            current_index = 0;
        }
    };

    class Stats {
    public:
        Stats() :
            n(0), total(0), sum_of_squares(0),
            min (numeric_limits<double>::max()),
            min1(numeric_limits<double>::max()),
            max (numeric_limits<double>::min()),
            max1(numeric_limits<double>::min()) {}
            
        double n;
        double total;
        double sum_of_squares;
        double min, min1;
        double max, max1;
        
        void collect(const double& value) {
            n++;
            total += value;
            sum_of_squares += value*value;
            
            if (value < min) min = value;
            else if (value < min1) min1 = value;
            
            if (value > max) max = value;
            else if (value > max1) max1 = value;
        }
        
        friend std::ostream& operator<< (std::ostream& o, Stats const& s) {
            o << " " << s.n
              << " " << s.total
              << " " << s.sum_of_squares
              << " " << s.min << " " << s.min
              << " " << s.max << " " << s.max;
            return o;
        }
    };

public:

    std::map<std::string, Stats> stats;
    
    void collect(const std::string& field, const double& value) {
        Stats& s = stats[field];
        s.n++;
        s.total += value;
        s.sum_of_squares += value*value;
    }
    
    void collect(const Message& message) {
        const Reflection* reflection = message.GetReflection();
        
        std::vector<const FieldDescriptor*> fields;
        reflection->ListFields(message, &fields);
        foreach (const FieldDescriptor* field, fields) {


            if (field->is_repeated()) {
                int count = reflection->FieldSize(message, field);
                for (int j = 0; j < count; j++) {
            
                    switch (field->cpp_type()) {
#define HANDLE_TYPE(CPPTYPE, METHOD)                                           \
                    case FieldDescriptor::CPPTYPE_##CPPTYPE:                           \
                        if (reflection->HasField(message, field))                      \
                            stats[field->full_name()].collect(reflection->GetRepeated##METHOD(message, field, j)); \
                        break;
                
                    // Only handle numeric types
                    HANDLE_TYPE(INT32 , Int32 );
                    HANDLE_TYPE(INT64 , Int64 );
                    HANDLE_TYPE(UINT32, UInt32);
                    HANDLE_TYPE(UINT64, UInt64);
                    HANDLE_TYPE(FLOAT , Float );
                    HANDLE_TYPE(DOUBLE, Double);
                    HANDLE_TYPE(BOOL  , Bool  );
#undef HANDLE_TYPE

                    case FieldDescriptor::CPPTYPE_MESSAGE:
                        collect(reflection->GetRepeatedMessage(message, field, j));
                        break;
                        
                    default:
                        break;
                    }
                }
            } else { // (not repeated)
            
                switch (field->cpp_type()) {
#define HANDLE_TYPE(CPPTYPE, METHOD)                                           \
                    case FieldDescriptor::CPPTYPE_##CPPTYPE:                           \
                        if (reflection->HasField(message, field))                      \
                            stats[field->full_name()].collect(reflection->Get##METHOD(message, field)); \
                        break;
                
                    // Only handle numeric types
                    HANDLE_TYPE(INT32 , Int32 );
                    HANDLE_TYPE(INT64 , Int64 );
                    HANDLE_TYPE(UINT32, UInt32);
                    HANDLE_TYPE(UINT64, UInt64);
                    HANDLE_TYPE(FLOAT , Float );
                    HANDLE_TYPE(DOUBLE, Double);
                    HANDLE_TYPE(BOOL  , Bool  );
#undef HANDLE_TYPE

                    case FieldDescriptor::CPPTYPE_MESSAGE:
                        collect(reflection->GetMessage(message, field));
                        break;
                        
                    default:
                        break;
                }
            }
        }
    }
    
    friend std::ostream& operator<< (std::ostream& o, StatsCollector const& sc) {
        typedef std::pair<std::string, Stats> p;
        
        ColumnSizeMeasurer csm;
        
        foreach (const p i, sc.stats)
            (csm << i.first << i.second).newline();
        
        foreach (const int& i, csm.lengths)
            o << i << " ";
        o << std::endl;
                
        foreach (const p i, sc.stats)
            o << i.first << i.second << std::endl;
        return o;
    }
};

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
    
    StatsCollector sc;
    
    foreach (std::string filename, input_files) {
        a4::io::A4Input in;
        in.add_file(filename);
        
        shared<a4::io::InputStream> stream = in.get_stream();
        // Stream in events we don't care about
        size_t i = 0;
        for (; i < event_index; i++)
            if (!stream->next())
                throw a4::Fatal("Ran out of events! There are only ", i, " on the file!");
                
        const size_t total = event_index + event_count;
        for (; i < total; i++) {
            a4::io::A4Message m = stream->next();
            if (!m)
                throw a4::Fatal("Ran out of events! There are only ", i, " on the file!");
            if (!collect_stats)
                dump_message(*m.message, variables);
            else
                sc.collect(*m.message);
        }
    }
    
    if (collect_stats)
        std::cout << sc;
}

