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
#include <boost/filesystem.hpp>

#include <google/protobuf/text_format.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
using google::protobuf::DescriptorPool;
using google::protobuf::FileDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Descriptor;
using google::protobuf::DescriptorProto;
using google::protobuf::Message;
using google::protobuf::Reflection;

#include <a4/input.h>
#include <a4/input_stream.h>
#include <a4/message.h>
#include <a4/dynamic_message.h>

#include <a4/io/A4Stream.pb.h>

using a4::io::A4Message;
using a4::io::FieldContent;

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
        
        size_t sub_size = 0, sub_fields = 0;
        
        std::vector<const FieldDescriptor*> fields;
        reflection->ListFields(message, &fields);
        foreach (const FieldDescriptor* field, fields)
            if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                sub_fields++;
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
        stats[desc->full_name() + ":bytes"].collect(this_size, this_only_size);
        stats[desc->full_name() + ":fields"].collect(fields.size() - sub_fields, 0);
        return this_size;
    }
    
    size_t total_bytes() {
        size_t sum = 0;
        foreach (auto& i, stats)
            sum += i.second.total_uniq;
        return sum;
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

void dump_message(const Message& message, bool shortform=true) {
    if (shortform) {
        std::cout << message.ShortDebugString() << std::endl;
    } else {
        VERBOSE("message of type '", message.GetDescriptor()->full_name(), "'");
        std::cout << message.DebugString() << std::endl;
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

bool dump_stream(shared<a4::io::InputStream> stream,
                  const bool show_footer,
                  const bool internal_msg,
                  const bool collect_stats,
                  const bool message_info,
                  const bool dump_all,
                  size_t& skip_events,
                  size_t& event_count,
                  const bool short_form,
                  const std::vector<Selection>& selections,
                  const bool show_metadata,
                  const bool dump_proto)
{
    
    if (show_footer) {
        foreach (auto& footer, stream->footers()) {
            // TODO(pwaller): Prettify
            VERBOSE("Footer: ", footer.DebugString());
        }
        return true;
    }
    
    if (show_metadata) {
        const auto& all_metadata = stream->all_metadata();
        
        VERBOSE("File contains ", all_metadata.size(), " header(s)");
        
        foreach (const auto& header, all_metadata) {
            foreach (const auto& metadata, header) {
                dump_message(*metadata->message(), short_form);
            }
        }
        
        stream->close();
        return true;
    }
    
    if (dump_proto) {
        const auto& file_descriptors = stream->get_filedescriptors();
        foreach (const auto* file_descriptor, file_descriptors) {
            auto& name = file_descriptor->name();
            if (name.find("google/") != std::string::npos)
                continue;
            
            VERBOSE("Proto: ", name);
            //std::cout << file_descriptor->DebugString();
            auto* evm = file_descriptor->FindMessageTypeByName("EventMetaData");
            if (evm) {
                
                DEBUG("Here: ", evm->name());
                std::cout << evm->DebugString();
                auto* f = evm->field(0);
                DEBUG("Field: ", f->DebugString());
                DEBUG("Options: ", f->options().uninterpreted_option().size());
                DEBUG("Options: ", f->options().unknown_fields().field_count    ());
                /*
                DescriptorProto x;
                ev->CopyTo(&x);
                DEBUG("here 1");
                std::cout << x.DebugString();
                */
            }
            
            //std::cout << file_descriptor->DebugString();
            auto* ev = file_descriptor->FindMessageTypeByName("Event");
            if (ev) {
                
                DEBUG("Here: ", ev->name());
                std::cout << ev->DebugString();
                auto* f = ev->field(0);
                DEBUG("Field: ", f->DebugString());
                DEBUG("Options: ", f->options().uninterpreted_option().size());
                DEBUG("Options: ", f->options().unknown_fields().field_count    ());
                /*
                DescriptorProto x;
                ev->CopyTo(&x);
                DEBUG("here 1");
                std::cout << x.DebugString();
                */
            }
        }
        
        stream->close();
        return true;
    }
    // Stream in events we don't care about
    for (; skip_events; skip_events--)
        if (internal_msg ? !stream->next_bare_message() : !stream->next())
            FATAL("Tried to read more events than there are in the file.");
    
    StatsCollector sc;
    MessageInfoCollector mic;
    
    for (; dump_all || event_count; event_count--) {
        shared<A4Message> m = internal_msg ? stream->next_bare_message() : stream->next();
        if (!m)
            break;
        
        // Skip messages which don't satisfy the selection
        if (any(selections, CheckSelection(*m->message())))
            continue;
        
        if (!(collect_stats || message_info))
            dump_message(*m->message(), short_form);
        
        if (collect_stats)
            sc.collect(*m->message());
        
        if (message_info)
            mic.collect(*m->message());
    }
    
    
    if (collect_stats)
        std::cout << sc;
    
    if (message_info) {
        std::cout << mic;
        std::cout << "Total message sizes: " << mic.total_bytes() << std::endl;
        std::cout << "Total bytes read (beware mmap, use nomm:// to avoid): " 
                  << stream->ByteCount() << std::endl;
    }
    
    stream->close();
    return true;
}

int main(int argc, char** argv) {
    a4::Fatal::enable_throw_on_segfault();

    namespace po = boost::program_options;

    std::vector<std::string> input_files, selection_strings;
    size_t event_count = 1, event_index = 0;
    bool collect_stats = false, short_form = false, message_info = false,
         internal_msg = false, dump_all = false, show_footer = false,
         show_metadata = false, dump_proto = false;
    
    DEBUG("argv[0] = ", argv[0]);
    
    std::string invocation(boost::filesystem::path(argv[0]).filename().string());
    
    if (invocation == "a4info")
        show_metadata = true;
    else if (invocation == "a4count")
        show_footer = true;
    
    
    po::positional_options_description p;
    p.add("input", -1);

    po::options_description commandline_options("Allowed options");
    commandline_options.add_options()
        ("help,h", "produce help message")
        ("event-index,i", po::value(&event_index), "event to start dumping from (starts at 0)")
        ("all,a", po::bool_switch(&dump_all), "dump all events")
        ("number,n", po::value(&event_count), "maximum number to dump")
        ("internal,I", po::bool_switch(&internal_msg), "also dump stream internal messages")
        ("collect-stats,S", po::bool_switch(&collect_stats), "should collect statistics for all numeric variables")
        ("message-info,M", po::bool_switch(&message_info), "should collect statistics relating to the message")
        ("short-form,s", po::bool_switch(&short_form), "print in a compact form, one event per line")
        ("select", po::value(&selection_strings), "Select messages by string equality (e.g. --select event_number:1234)")
        ("footer,f", po::bool_switch(&show_footer), "Show information from the footer (e.g. object counts)")
        ("metadata,m", po::bool_switch(&show_metadata), "Show only metadata")
        ("proto,p", po::bool_switch(&dump_proto), "Dump .proto files")
        ("input", po::value(&input_files), "input file names (runs once per specified file)")
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
    
    if (show_footer || dump_all) {
        FATAL_ASSERT((event_count == 1 && event_index == 0), "Specifying event count or index is incompatible with --footer and --dump-all");
    }
    
    a4::io::A4Input in;
    
    foreach (std::string filename, input_files)
        in.add_file(filename);
        
    std::vector<Selection> selections;
    foreach (auto& selection_string, selection_strings)
        selections.push_back(Selection(selection_string));
        
    while (shared<a4::io::InputStream> stream = in.get_stream()) {
        if (!dump_stream(stream, show_footer, internal_msg, collect_stats, 
                         message_info, dump_all, event_index, event_count, 
                         short_form, selections, show_metadata, dump_proto))
            return 1;
    }
    
    return 0;
}

