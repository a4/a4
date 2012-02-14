#include <algorithm>

#include <limits>
using std::numeric_limits;

#include <utility>
#include <string>

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
using google::protobuf::Descriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

#include <a4/input.h>
#include <a4/message.h>
#include <dynamic_message.h>


template<typename T> struct ItemOrdering {
    bool operator() (const T& lhs, const T& rhs) { 
        return lhs.first < rhs.first;
        // return lhs.first->full_name() < rhs.first->full_name();
    }
};

/// A class to measure the largest column width and then print an aligned
/// table. Two passes need to be made over the data, one to measure,
/// another to print
class ColumnSizeMeasurer {
public:
    ColumnSizeMeasurer() : current_index(0), measuring(true) {}

    std::vector<int> lengths;
    size_t current_index;
    bool measuring;
    
    void print(std::ostream& out) {}
    
    template<typename T, class... Args>
    void print(std::ostream& out, const T& value, const Args& ...args) {
        if (measuring) {
        
            std::stringstream stream;
            stream << std::fixed << value;
            const int this_length = stream.tellp();
            
            if (current_index >= lengths.size())
                lengths.push_back(this_length);
            else {
                if (lengths.at(current_index) < this_length)
                    lengths.at(current_index) = this_length;
            }
            
        } else
            out << std::setw(lengths.at(current_index)) << std::fixed << value << " ";
        
        current_index++;
        print(out, args...);
    }
    
    void newline(std::ostream& out) {
        current_index = 0;
        if (!measuring)
            out << std::endl;
    }
};

/// Collect stats about one variable
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
    double total_uniq;
    double sum_of_squares;
    double min, min1;
    double max, max1;
    
    /// Collect one value
    /// Values aren't counted if they are the minimum or maximum. This is to
    /// avoid pulling the distribution towards any default values.
    void collect(const double& value) {
    
        if (value == -999 || value == -9999 || value == -99999)
            // Special case.
            return;
    
        // (or equal) to make sure don't follow second branch otherwise.
        if (value <= min) min = value;
        else if (value < min1) min1 = value;
        
        if (value >= max) max = value;
        else if (value > max1) max1 = value;
        
        // This if statement used to prevent the min/max from being included in 
        // the totals and counts, but no longer.
        
        // If min1/max1 aren't set, then we haven't seen many interesting 
        // values. Therefore we should include this value in the count.            
        //if ((value != min && value != max) 
            //|| min1 == numeric_limits<double>::max() 
            //|| max1 == numeric_limits<double>::min()) {
        n++;
        total += value;
        sum_of_squares += value*value;
        //}
    }
    
    void collect(const double value, const double uniq) {
        collect(value);
        total_uniq += uniq;
    }
    
    double mean() const {
        return total / n;
    }
    
    double stddev() const {
        return sqrt(sum_of_squares/n - (mean()*mean()));
    }
    
    void print(std::ostream& out, ColumnSizeMeasurer& csm) {
        if (n == 0) {
            csm.print(out, 0, 0, 0, 0, 0, 0, 0, 0);
            return;
        }
        csm.print(out, n, total, mean(), stddev(), min, max, 
                  (min1 == numeric_limits<double>::max() ? min : min1), 
                  (max1 == numeric_limits<double>::min() ? max : max1));
    }
};

/// Collect statistics about variables
class StatsCollector {
    
public:
    std::unordered_map<const FieldDescriptor*, Stats> stats;
    
    /// Collect values from one message
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
                        stats[field].collect(reflection->GetRepeated##METHOD(message, field, j)); \
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
                            stats[field].collect(reflection->Get##METHOD(message, field)); \
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
        typedef std::pair<const FieldDescriptor*, Stats> StatsItem;
        
        std::vector<StatsItem> sorted_stats;
        
        foreach (StatsItem i, sc.stats)
            sorted_stats.push_back(i);            
        
                
        std::sort(sorted_stats.begin(), sorted_stats.end(), ItemOrdering<StatsItem>());
        
        ColumnSizeMeasurer csm;
        
        // Do two iterations, one to measure, other to print.
        for (int i = 0; i < 2; i++) {
            csm.print(o, "Variable", "n", "total", "mean", "stddev", "min", "max", "min1", "max1");
            csm.newline(o);
        
            foreach (StatsItem i, sorted_stats) {
                o << std::left; // left align first column
                csm.print(o, i.first->full_name());
                o << std::right;
                i.second.print(o, csm);
                csm.newline(o);
            }
        
            csm.measuring = false;
        }
        
        return o;
    }
};

class MessageInfoCollector {
public:
    std::map<std::string, Stats> stats;
    
    MessageInfoCollector() {}

    /// Collect values from one message
    size_t collect(const Message& message) {
        const Reflection* reflection = message.GetReflection();
        const auto* desc = message.GetDescriptor();
        
        size_t sub_size = 0;
        
        std::vector<const FieldDescriptor*> fields;
        reflection->ListFields(message, &fields);
        foreach (const FieldDescriptor* field, fields)
            if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                if (field->is_repeated()) {
                    int count = reflection->FieldSize(message, field);
                    for (int j = 0; j < count; j++)
                        sub_size += collect(reflection->GetRepeatedMessage(message, field, j));
                } else { // (not repeated)
                    sub_size += collect(reflection->GetMessage(message, field));
                }
            }
        
        std::string data;
        message.SerializeToString(&data);
        auto this_size = data.size();
        auto this_only_size = this_size - sub_size;
        stats[desc->full_name()].collect(this_size, this_only_size);
        return this_size;
    }
        
    friend std::ostream& operator<< (std::ostream& o, MessageInfoCollector const& mic) {
        typedef std::pair<std::string, Stats> StatsItem;
        
        std::vector<StatsItem> sorted_stats;
        
        foreach (StatsItem i, mic.stats)
            sorted_stats.push_back(i);
        
        std::sort(sorted_stats.begin(), sorted_stats.end(), ItemOrdering<StatsItem>());
        
        ColumnSizeMeasurer csm;
        
        // Do two iterations, one to measure, other to print.
        for (int i = 0; i < 2; i++) {
            csm.print(o, "Variable", "total_uniq", "n", "total", "mean", "stddev", "min", "max", "min1", "max1");
            csm.newline(o);
        
            foreach (StatsItem i, sorted_stats) {
                o << std::left; // left align first column
                csm.print(o, i.first);
                o << std::right;
                csm.print(o, i.second.total_uniq);
                i.second.print(o, csm);
                csm.newline(o);
            }
        
            csm.measuring = false;
        }
        
        return o;
    }
    
};

void dump_message(const Message& message, 
                  const std::vector<std::string>& vars, 
                  const std::vector<std::string>& types, bool shortform=true) {
                  
    if (vars.size()) {
        // specialized code
        FATAL("Not implemented yet");
    } else {
        std::string str, type(message.GetDescriptor()->name());
        if (types.size() && find(types.begin(), types.end(), type) == types.end()) {
            // Not a selected type
            return;
        }
        if (shortform) {
            std::cout << message.ShortDebugString() << std::endl;
        } else {
            std::cout << "message of type '" << type << "':" <<std::endl;
            google::protobuf::TextFormat::PrintToString(message, &str);
            std::cout << str << std::endl;
        }
    }    
}

template<class Container, class Predicate>
bool any(const Container& container, const Predicate& pred) {
    foreach (const auto& item, container)
        if (pred(item))
            return true;
    return false;
}

class Selection
{
public:
    Selection(const std::string& sel) : _desc(NULL), _field_desc(NULL) {
        auto pos = sel.find(":");
        _name = sel.substr(0, pos);
        _value = sel.substr(pos+1);
    }
    
    bool check(const Message& m) const {
        if (_desc != m.GetDescriptor()) {
            // Optimisation so we don't do the lookup every time
            _desc = m.GetDescriptor();
            _field_desc = _desc->FindFieldByName(_name);
            if (!_field_desc)
                FATAL("Unknown field name: ", _name);
        }
        // HACK: Inefficient string comparison because it's the quickest implementation
        return FieldContent(m, _field_desc).str() == _value;
    }
    
    std::string _name, _value;
    
    mutable const Descriptor* _desc;
    mutable const FieldDescriptor* _field_desc;
};

class CheckSelection {
    const Message& _m;
public:    
    CheckSelection(const Message& m) : _m(m) {}
    bool operator()(const Selection& s) const {
        return !s.check(_m);
    }
};

int main(int argc, char ** argv) {
    a4::Fatal::enable_throw_on_segfault();

    namespace po = boost::program_options;

    std::vector<std::string> input_files, variables, types, selection_strings;
    size_t event_count = -1, event_index = -1;
    bool collect_stats = false, short_form = false, message_info = false,
         stream_msg = false, dump_all = false;
    
    po::positional_options_description p;
    p.add("input", -1);

    po::options_description commandline_options("Allowed options");
    commandline_options.add_options()
        ("help,h", "produce help message")
        ("event-index,i", po::value(&event_index)->default_value(0), "event to start dumping from (starts at 0)")
        ("count,c", po::value(&event_count)->default_value(1), "maximum number to dump")
        ("all,a", po::bool_switch(&dump_all)->default_value(false), "dump all events")
        ("input", po::value(&input_files), "input file names (runs once per specified file)")
        ("var,v", po::value(&variables), "variables to dump (defaults to all)")
        ("type,t", po::value(&types), "variables to dump (defaults to all)")
        ("stream,s", po::bool_switch(&stream_msg)->default_value(false), "also dump stream internal messages")
        ("collect-stats,S", po::value(&collect_stats), "should collect statistics for all numeric variables")
        ("message-info", po::bool_switch(&message_info)->default_value(false), "should collect statistics relating to the message")
        ("short-form", po::bool_switch(&short_form)->default_value(false), "print in a compact form, one event per line")
        ("select", po::value(&selection_strings), "Select messages by string equality (e.g. --select event_number:1234)")
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
    MessageInfoCollector mic;
    
    a4::io::A4Input in;
    
    foreach (std::string filename, input_files)
        in.add_file(filename);
        
    shared<a4::io::InputStream> stream = in.get_stream();
    // Stream in events we don't care about
    size_t i = 0;
    for (; i < event_index; i++)
        if (stream_msg ? !stream->next_bare_message() : !stream->next())
            FATAL("Ran out of events! There are only ", i, " on the file!");
        
    std::vector<Selection> selections;
    foreach (auto& selection_string, selection_strings)
        selections.push_back(Selection(selection_string));
    
    const size_t total = event_index + event_count;
    for (; dump_all || i < total; i++) {
        a4::io::A4Message m = stream_msg ? stream->next_bare_message() : stream->next();
        if (!m) break;
        
        // Skip messages which don't satisfy the selection
        if (any(selections, CheckSelection(*m.message())))
            continue;
        
        if (!(collect_stats || message_info))
            dump_message(*m.message(), variables, types, short_form);
        
        if (collect_stats)
            sc.collect(*m.message());
        
        if (message_info)
            mic.collect(*m.message());
    }
    
    if (collect_stats)
        std::cout << sc;
    
    if (message_info)
        std::cout << mic;
}

