#include <boost/foreach.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fstream>

#include "a4/results.h"
#include "a4/writer.h"
#include "a4/reader.h"

class A4Key : public StreamableTo<a4::io::A4Key, A4Key> {
    public:
        A4Key() {};
        A4Key(string name) : name(name) {};
        virtual void to_pb() const { pb.reset(new a4::io::A4Key()); pb->set_name(name); }
        virtual void from_pb() { name = pb->name(); }
        string name;
};

using namespace std;

void Results::to_file(std::string fn) {
    auto w = new Writer(fn, "Results");
    BOOST_FOREACH(string n, list<Streamable>()) {
        A4Key key(n);
        w->write(key);
        w->write(*get_checked<Streamable>(n));
    }
    if (metadata) w->metadata(*metadata);
    delete w;
}

std::vector<Results *> Results::from_file(std::string fn) {
    auto r = boost::shared_ptr<Reader>(new Reader(fn));
    std::vector<Results *> resv;
    Results * res = NULL;
    while (r->is_good()) {
        ReadResult keyres = r->read();
        if (keyres.class_id == A4Key::class_id) {
            auto key = boost::dynamic_pointer_cast<A4Key>(keyres.object);
            ReadResult rr = r->read();
            if (!rr.object) throw "up";
            auto obj = boost::dynamic_pointer_cast<Printable>(rr.object);
            if (!res) res = new Results();
            res->set(key->name, obj);
        } else if (boost::dynamic_pointer_cast<MetaData>(keyres.object)) {
            if (!res) res = new Results();
            res->metadata = boost::dynamic_pointer_cast<MetaData>(keyres.object);
            resv.push_back(res);
            res = NULL;
        }
        if (keyres == READ_ERROR) throw "up";
    }
    if (res) resv.push_back(res);
    return resv;
}

Results & Results::__add__(const Addable & _source) {
    const Results & source = dynamic_cast<const Results&>(_source);
    BOOST_FOREACH(string n, list<Addable>()) {
        boost::shared_ptr<Addable> other = source.get_checked<Addable>(n);
        if (other) get_checked<Addable>(n)->__add__(*other);
    }
    BOOST_FOREACH(string n, source.list<Addable>()) {
        boost::shared_ptr<Addable> self = get_checked<Addable>(n);
        if (!self) set(n, reinterpret_cast<Printable*>(source.get_checked<Cloneable>(n)->clone()));
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
        results->set(n, reinterpret_cast<Printable*>(get_checked<Cloneable>(n)->clone()));
    return results;
};

std::string Results::__repr__() {
    std::stringstream ss;
    ss << Printable::__repr__() << ", containing " << list().size() << " objects";
    return ss.str();
}

std::string Results::__str__() {
    std::stringstream ss;
    ss << Printable::__str__() << std::endl;
    BOOST_FOREACH(string n, list<Printable>())
        ss << " * " << n << " : " << get_checked<Printable>(n)->__repr__() << std::endl;
    return ss.str();
}

