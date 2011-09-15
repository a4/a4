#include <functional>
#include <thread>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include <a4/input.h>

using namespace a4::io;

typedef std::unique_lock<std::mutex> Lock;
//typedef boost::unique_lock<boost::mutex> Lock;

A4Input::A4Input(std::string name) {};

/// Add a stream to be processed, Returns this object again.
A4Input & A4Input::add_stream(shared<A4InputStream> s) {
    _streams.push_back(s);
    _ready.push_front(s.get());
};

/// Add a file to be processed, Returns this object again.
A4Input & A4Input::add_file(const std::string & filename) {
    auto s = shared<A4InputStream>(new A4InputStream(filename));
    _streams.push_back(s);
    _ready.push_front(s.get());
};

void A4Input::report_finished(A4Input * input, A4InputStream* _s) {
    Lock l2(input->_mutex);
    if (_s->end()) {
        assert(input->_processing.erase(_s) == 1);
        input->_finished.insert(_s);
    }
}

/// Get a stream for processing, returns NULL if none are left (threadsafe).
/// Optionally give your "name" (for debugging)
shared<A4InputStream> A4Input::get_stream() {
    Lock lock(_mutex);
    if (_ready.empty()) return shared<A4InputStream>();
    A4InputStream * s = _ready.back();
    _ready.pop_back();

    _processing.insert(s);

    //auto ret = shared<A4InputStream>(s, (new Callback(this))->Call);
    std::function<void (A4InputStream*)> cb = std::bind(&A4Input::report_finished, this, std::placeholders::_1);
    auto ret = shared<A4InputStream>(s, cb);
    return ret;
}

