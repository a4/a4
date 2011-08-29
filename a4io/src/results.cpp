#include <boost/foreach.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fstream>

#include "a4/results.h"
#include "a4/writer.h"
#include "a4/reader.h"
//#include "a4/reader_impl.h"

using namespace std;

void Results::to_file(std::string fn) {
    auto w = new Writer(fn, "Results");
    BOOST_FOREACH(string n, list<Streamable>())
        w->write(*get_checked<Streamable>(n));
    if (metadata) w->metadata(*metadata);
    delete w;
}

Results * Results::from_file(std::string fn) {
    //auto r = new Reader(fn);
    
    //::google::protobuf::io::IstreamInputStream raw_in(&_input);
    //::google::protobuf::io::CodedInputStream coded_in(&raw_in);
    //a4::io::Results rpb;
    ///rpb.ParseFromIstream(&coded_in);
    //rpb.ParseFromIstream(&_input);
    //return ResultsPtr(new Results(rpb));
    return NULL;
}

Results & Results::__add__(const Addable & _source) {
    const Results & source = dynamic_cast<const Results&>(_source);
    BOOST_FOREACH(string n, list<Addable>()) {
        boost::shared_ptr<Addable> other = source.get_checked<Addable>(n);
        if (other) get_checked<Addable>(n)->__add__(*other);
    }
    BOOST_FOREACH(string n, source.list<Addable>()) {
        boost::shared_ptr<Addable> self = get_checked<Addable>(n);
        if (!self) set<Named>(n, reinterpret_cast<Named*>(source.get_checked<Cloneable>(n)->clone()));
    }
    return *this;
}

Results & Results::__mul__(const double & weight) {
    BOOST_FOREACH(string n, list<Scalable>()) *get_checked<Scalable>(n) *= weight;
    return *this;
}

Results * Results::clone() const {
    Results * results = new Results();
    BOOST_FOREACH(string n, list<Cloneable>())
        results->set<Named>(n, reinterpret_cast<Named*>(get_checked<Cloneable>(n)->clone()));
    return results;
};

std::string Results::__repr__() {
    std::stringstream ss;
    ss << Named::__repr__() << ", containing " << list().size() << " objects";
    return ss.str();
}

std::string Results::__str__() {
    std::stringstream ss;
    ss << Named::__str__() << std::endl;
    BOOST_FOREACH(string n, list<Printable>())
        ss << " * " << get_checked<Printable>(n)->__repr__() << std::endl;
    return ss.str();
}

