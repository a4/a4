#include <iostream>
#include <cstdlib>
#include <fstream>

#include <sys/stat.h>

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
            throw a4::Fatal("output_file exists and is a directory!");
        
        if (S_ISLNK(buffer.st_mode)) {
            // We can call lstat() here to determine the properties of the linkee
            // But I'm not sure exactly what the right thing to do is.
            throw a4::Fatal("Destination is a link. Not yet implemented, see "
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

/// Create a new output stream. For each call we create an independent output
/// stream pointing to a different file, unless the destination is a fifo, in
/// which case we only allow one call to `get_stream()`.
shared<OutputStream> A4Output::get_stream() {
    Lock lock(_mutex);
    int count = _filenames.size() + 1;
    
    std::string filename;
    
    if (!_regular_file) {
        // Destination is non-regular so we should write to it directly.
        if (count != 1)
            throw a4::Fatal("Can only create one output stream on a fifo output");
        filename = _output_file;
    } else {
        filename = _output_file + "." + boost::lexical_cast<std::string>(count);
    }
    
    shared<OutputStream> os(new OutputStream(filename, _description));
    _out_streams.push_back(os);
    _filenames.push_back(filename);
    return os;
}

/// Close all output streams and merge their contents together.
/// If there was only one output stream, we rename it to the destination file
/// If the output is a fifo, we do nothing other than close the output stream
bool A4Output::close() {
    Lock lock(_mutex);
    _closed = true;
    
    foreach(shared<OutputStream> s, _out_streams) {
        if (s->opened()) s->close();
    }
    
    if (!_regular_file) {
        // We're writing to exactly one output, no filename fixup to do.
        return true;
    }
    
    if (_out_streams.size() == 1 && _out_streams[0]->opened()) {
        const char * from = _filenames[0].c_str();
        const char * to = _output_file.c_str();
        unlink(to);
        if (rename(from, to) == -1) {
            std::cerr << "ERROR - a4::io:A4Output - Can not rename " << from << " to " << to << "!" << std::endl;
            return false;
        };
    } else if (_out_streams.size() > 1) {
        // concatenate all output files
        std::fstream out(_output_file.c_str(), ios::out | ios::trunc | ios::binary);
        if (!out) {
            std::cerr << "ERROR - a4::io:A4Output - Could not open " << _output_file << " for writing!" << std::endl;
            return false;
        }
        for (uint32_t i = 0; i < _filenames.size(); i++) {
            std::string fn = _filenames[i];
            shared<OutputStream> ostream = _out_streams[i];
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
