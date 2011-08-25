#include "a4/results.h"
#include "a4/proto/io/Results.pb.h"

#include <boost/foreach.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fstream>

using namespace std;
using namespace a4::io;

MessagePtr Results::get_message() {
    boost::shared_ptr<a4pb::Results> res(new a4pb::Results);
    if (title.size() > 0) res->set_title(title);
    BOOST_FOREACH(string n, list<Cutflow>()) {
        a4pb::Cutflow * cf = res->add_cutflow();
        cf->CopyFrom(*get<Cutflow>(n)->get_message());
        cf->set_name(n);
    }
    BOOST_FOREACH(string n, list<H1>()) {
        a4pb::Histogram1 * cf = res->add_h1();
        cf->CopyFrom(*get<H1>(n)->get_message());
        cf->set_name(n);
    }
    BOOST_FOREACH(string n, list<H2>()) {
        a4pb::Histogram2 * cf = res->add_h2();
        cf->CopyFrom(*get<H2>(n)->get_message());
        cf->set_name(n);
    }
    return res;
}

Results::Results(Message& m) {
    a4pb::Results * msg = dynamic_cast<a4pb::Results*>(&m);
    if (msg->has_title()) title = msg->title();
    BOOST_FOREACH(a4pb::Cutflow cf, msg->cutflow()) set<Cutflow>(cf.name(), new Cutflow(cf));
    BOOST_FOREACH(a4pb::Histogram1 h1, msg->h1()) set<H1>(h1.name(), new H1(h1));
    BOOST_FOREACH(a4pb::Histogram2 h2, msg->h2()) set<H2>(h2.name(), new H2(h2));
}

Results::~Results()
{
}

void Results::to_file(std::string fn) {
    std::fstream _output(fn.c_str(), ios::out | ios::trunc | ios::binary);
    //::google::protobuf::io::OstreamOutputStream _raw_out(&_output);
    //::google::protobuf::io::CodedOutputStream _coded_out(&_raw_out);
    get_message()->SerializeToOstream(&_output);
}

ResultsPtr Results::from_file(std::string fn) {
    fstream _input(fn.c_str(), ios::in | ios::binary);
    //::google::protobuf::io::IstreamInputStream raw_in(&_input);
    //::google::protobuf::io::CodedInputStream coded_in(&raw_in);
    a4pb::Results rpb;
    //rpb.ParseFromIstream(&coded_in);
    rpb.ParseFromIstream(&_input);
    return ResultsPtr(new Results(rpb));
}

BinnedData & Results::__add__(const BinnedData & _source) {
    const Results & source = dynamic_cast<const Results&>(_source);
    BOOST_FOREACH(string n, list<Cutflow>()) {
        CutflowPtr other = source.get_checked<Cutflow>(n);
        if (other) get<Cutflow>(n)->__add__(*other);
    }
    BOOST_FOREACH(string n, source.list<Cutflow>()) {
        CutflowPtr self = get_checked<Cutflow>(n);
        if (!self) set<Cutflow>(n, new Cutflow(*source.get_checked<Cutflow>(n)));
    }
    BOOST_FOREACH(string n, list<H1>()) {
        H1Ptr other = source.get_checked<H1>(n);
        if (other) get<H1>(n)->__add__(*other);
    }
    BOOST_FOREACH(string n, source.list<H1>()) {
        H1Ptr self = get_checked<H1>(n);
        if (!self) set<H1>(n, new H1(*source.get_checked<H1>(n)));
    }
    BOOST_FOREACH(string n, list<H2>()) {
        H2Ptr other = source.get_checked<H2>(n);
        if (other) get<H2>(n)->__add__(*other);
    }
    BOOST_FOREACH(string n, source.list<H2>()) {
        H2Ptr self = get_checked<H2>(n);
        if (!self) set<H2>(n, new H2(*source.get_checked<H2>(n)));
    }
    return *this;
}

BinnedData & Results::__mul__(const double & weight) {
    BOOST_FOREACH(string n, list<Cutflow>()) *get<Cutflow>(n) *= weight;
    BOOST_FOREACH(string n, list<H1>()) *get<H1>(n) *= weight;
    BOOST_FOREACH(string n, list<H2>()) *get<H2>(n) *= weight;
    return *this;
}

void Results::print(std::ostream & out) const {
    BOOST_FOREACH(string n, list<Cutflow>()) {
        out << "Cutflow '" << n << "':" << endl;
        get_checked<Cutflow>(n)->print(out);
    }
}

