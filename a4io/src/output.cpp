#include <iostream>
#include <cstdlib>
#include <fstream>

#include <thread>
#include <boost/lexical_cast.hpp>

#include <a4/output.h>

using namespace a4::io;
using std::ios;

typedef std::unique_lock<std::mutex> Lock;

A4Output::A4Output(std::string output_file, std::string description) :
    output_file(output_file),
    description(description),
    _closed(false)
{
};

A4Output::~A4Output() {
    if (!_closed) close();
}

shared<A4OutputStream> A4Output::get_stream() {
    Lock lock(_mutex);
    int count = _filenames.size() + 1;
    std::string fn = output_file + "." + boost::lexical_cast<std::string>(count);
    shared<A4OutputStream> os(new A4OutputStream(fn, description));
    _out_streams.push_back(os);
    _filenames.push_back(fn);
    return os;
}

bool A4Output::close() {
    Lock lock(_mutex);
    _closed = true;
    for (auto s = _out_streams.begin(), end = _out_streams.end(); s != end; s++) {
        (*s)->close();
    }
    if (_out_streams.size() == 1) {
        auto from = _filenames[0].c_str();
        auto to = output_file.c_str();
        unlink(to);
        if (rename(from, to) == -1) {
            std::cerr << "ERROR - a4::io:A4Output - Can not rename " << from << " to " << to << "!" << std::endl;
            return false;
        };
    } else if (_out_streams.size() > 1) {
        // concatenate all output files
        std::fstream out(output_file.c_str(), ios::out | ios::trunc | ios::binary);
        if (!out) {
            std::cerr << "ERROR - a4::io:A4Output - Could not open " << output_file << " for writing!" << std::endl;
            return false;
        }
        for (int i = 0; i < _filenames.size(); i++) {
            std::string fn = _filenames[i];
            auto ostream = _out_streams[i];
            if (!ostream->opened()) continue; // maybe threads did not use their output
            std::fstream in(fn.c_str(), ios::in | ios::binary);
            if (!in) {
                std::cerr << "ERROR - a4::io:A4Output - Could not open " << fn << "! Maybe some threads failed?" << std::endl;
                return false;
            }
            out << in.rdbuf();
            unlink(fn.c_str());
        }
    }
    return true;
}
