#include <functional>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include <a4/input.h>

using namespace a4::io;

typedef boost::unique_lock<boost::mutex> Lock;
//typedef boost::unique_lock<boost::mutex> Lock;

A4Input::A4Input(std::string name) {};

/// Add a stream to be processed, Returns this object again.
A4Input & A4Input::add_stream(shared<InputStream> s) {
    _streams.push_back(s);
    _ready.push_front(s.get());
    return *this;
};

/// Add a file to be processed, Returns this object again.
A4Input & A4Input::add_file(const std::string & filename) {
    shared<InputStream> s(new InputStream(filename));
    _streams.push_back(s);
    _ready.push_front(s.get());
    return *this;
};

void A4Input::report_finished(A4Input * input, InputStream* _s) {
    Lock l2(input->_mutex);
    if (_s->end()) {
        assert(input->_processing.erase(_s) == 1);
        input->_finished.insert(_s);
        std::cerr << "Finished processing " << _s->str() << std::endl;
    } else if (_s->error() || input->_resched_count[_s] > 0) {
        std::cerr << "ERROR - a4::io::A4Input - '" << _s->str() << "' encountered an error during reading!" << std::endl;
        input->_error.insert(_s);
    } else {
        std::cerr << "Stream " << _s->str() << " came back incompletely processed! Rescheduling..." << std::endl;
        input->_ready.push_front(_s);
        input->_resched_count[_s]++;
    }
}

/// Get a stream for processing, returns NULL if none are left (threadsafe).
/// Optionally give your "name" (for debugging)
shared<InputStream> A4Input::get_stream() {
    Lock lock(_mutex);
    if (_ready.empty()) return shared<InputStream>();
    InputStream * s = _ready.back();
    _ready.pop_back();

    _processing.insert(s);

    //auto ret = shared<InputStream>(s, (new Callback(this))->Call);
    std::function<void (InputStream*)> cb = std::bind(&A4Input::report_finished, this, std::placeholders::_1);
    shared<InputStream> ret(s, cb);
    std::cerr << "Starting to process " << s->str() << std::endl;
    return ret;
}

