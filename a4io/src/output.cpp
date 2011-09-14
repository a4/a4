#include <iostream>
#include <fstream>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <a4/output.h>

using namespace a4::io;
using std::ios;

A4Output::A4Output(std::string output_file, std::string description) :
    output_file(output_file),
    description(description)
{

};


shared<A4OutputStream> A4Output::get_stream(
        uint32_t content_class_id,
        uint32_t metadata_class_id,
        bool metadata_refers_forward=false,
        bool compression=true) 
{
    int count = _filenames.size() + 1;
    std::string fn = output_file + "." + boost::lexical_cast<std::string>(count);
    shared<A4OutputStream> os(new A4OutputStream(
                fn,
                description, 
                content_class_id, 
                metadata_class_id, 
                metadata_refers_forward, 
                compression));
    _out_streams.push_back(os);
    _filenames.push_back(fn);
    return os;
}

bool A4Output::close() {
    for (auto s = _out_streams.begin(), end = _out_streams.end(); s != end; s++) {
        (*s)->close();
    }
    if (_out_streams.size() == 1) {
        auto from = boost::filesystem::path(_filenames[0]);
        auto to = boost::filesystem::path(output_file);
        boost::filesystem::rename(from, to);
    } else if (_out_streams.size() > 1) {
        // concatenate all output files
        std::fstream out(output_file.c_str(), ios::out | ios::trunc | ios::binary);
        for (auto fn = _filenames.begin(), end = _filenames.end(); fn != end; fn++) {
            std::fstream in((*fn).c_str(), ios::in | ios::binary);
            out << in.rdbuf();
            boost::filesystem::remove(boost::filesystem::path(*fn));
        }
    }
}
