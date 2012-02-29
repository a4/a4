#include <iostream>
#include <cstdlib>
#include <fstream>

#include <sys/stat.h>

#include <a4/types.h>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include <a4/output.h>

using namespace a4::io;
using std::ios;

typedef boost::unique_lock<boost::mutex> Lock;

A4Output::A4Output(std::string output_file, std::string description) :
    _closed(false),
    _regular_file(true),
    _output_file(output_file),
    _description(description)
{
    struct stat buffer;
    
    if (_output_file == "-") {
        _output_file = "/dev/stdout";
        _regular_file = false;
        
    } else if (stat(output_file.c_str(), &buffer) != -1) {
        // File already exists
        if (S_ISDIR(buffer.st_mode))
            FATAL("output_file exists and is a directory!");
        
        if (S_ISLNK(buffer.st_mode)) {
            // We can call lstat() here to determine the properties of the linkee
            // But I'm not sure exactly what the right thing to do is.
            FATAL("Destination is a link. Not yet implemented, see "
                             "https://github.com/JohannesEbke/a4/issues/39");
        }
        
        if (!S_ISREG(buffer.st_mode)) {
            // Not a regular file: could be a socket, block, character or fifo 
            // "device".
            _regular_file = false;
        }
    }
}

A4Output::~A4Output() {
    if (!_closed) close();
}

void A4Output::report_finished(A4Output* output, OutputStream* s) {
    if (s->opened()) 
        s->close();
}

/// Create a new output stream. For each call we create an independent output
/// stream pointing to a different file, unless the destination is a fifo, in
/// which case we only allow one call to `get_stream()`.
shared<OutputStream> A4Output::get_stream(std::string postfix, bool forward_metadata) {
    Lock lock(_mutex);
    int count = _filenames[postfix].size() + 1;
    
    // Determine filename
    std::string filename; 
    if (!_regular_file) {
        // Destination is non-regular so we should write to it directly.
        if (count != 1)
            FATAL("Can only create one output stream on a fifo output");
        if (postfix != "")
            FATAL("Cannot split output stream on a fifo output");
        filename = _output_file;
    } else {
        std::string number = boost::lexical_cast<std::string>(count);
        if (postfix == "") {
            filename = _output_file + "." + number;
        } else {
            filename = _output_file + "." + postfix + "." + number;
        }
    }
    
    shared<OutputStream> os(new OutputStream(filename, _description));
    
    if (forward_metadata)
        os->set_forward_metadata();
    
    _out_streams[postfix].push_back(os);
    _filenames[postfix].push_back(filename);

    if (!_regular_file) os->open();

    #if defined(HAVE_LAMBDA) and defined(HAVE_NOEXCEPT)
    auto cb = [&](OutputStream* x) noexcept { report_finished(this, x); };
    #else
    std::function<void (OutputStream*)> cb =
        std::bind(&A4Output::report_finished, this, std::placeholders::_1);
    #endif
    
    shared<OutputStream> ret(os.get(), cb);
    return ret;
}

/// Close all output streams and merge their contents together.
/// If there was only one output stream, we rename it to the destination file
/// If the output is a fifo, we do nothing other than close the output stream
bool A4Output::close() {
    Lock lock(_mutex);
    _closed = true;
    
    foreach(auto postfix, _out_streams)
        foreach(auto s, postfix.second)
            if (s->opened()) 
                s->close();
    
    if (!_regular_file) {
        // We're writing to exactly one output, no filename fixup to do.
        return true;
    }

    bool success = true;
    foreach(auto postfix, _out_streams) {
        std::vector<std::string> used_streams;
        for(uint32_t i = 0; i < postfix.second.size(); i++) {
            if (postfix.second[i]->opened()) 
                used_streams.push_back(_filenames[postfix.first][i]);
        }
        std::string out = _output_file;
        if (postfix.first != "")
            out += "." + postfix.first;
        success = success && concatenate(used_streams, out);
    }
    return success;
}

bool A4Output::concatenate(const std::vector<std::string>& filenames, 
                           const std::string target) {
    if (filenames.size() == 1) {
        const char* from = filenames[0].c_str();
        const char* to = target.c_str();
        unlink(to);
        if (rename(from, to) == -1) {
            ERROR("Cannot rename ", from, " to ", to, "!");
            return false;
        }
    } else if (filenames.size() > 1) {
        // concatenate all output files
        std::fstream out(target.c_str(), ios::out | ios::trunc | ios::binary);
        if (!out) {
            ERROR("Could not open ", target, " for writing!");
            return false;
        }
        for (uint32_t i = 0; i < filenames.size(); i++) {
            std::string fn = filenames[i];
            std::fstream in(fn.c_str(), ios::in | ios::binary);
            if (!in) {
                ERROR("Could not open ", fn, "! Maybe some threads failed?");
                return false;
            }
            out << in.rdbuf();
            unlink(fn.c_str());
        }
    }
    return true;
}
