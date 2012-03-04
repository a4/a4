#include <a4/input_stream.h>
#include <a4/message.h>

#include "input_stream_impl.h"

#include <a4/io/A4Stream.pb.h>

namespace a4 {
namespace io {

    // Forward everything to the implementation object.
    InputStream::InputStream(std::string url) {
        _impl.reset(new InputStreamImpl(resource_from_url(url), url));
    }

    InputStream::InputStream(UNIQUE<InputStreamImpl> impl) {
        _impl = std::move(impl);
    }
    
    InputStream::~InputStream() {}
    
    shared<A4Message> InputStream::next() { return _impl->next(); }
    
    shared<A4Message> InputStream::next_with_metadata() {
        return _impl->next_with_metadata();
    
    }
    shared<A4Message> InputStream::next_bare_message() {
        return _impl->next_bare_message();
    }
    shared<const A4Message> InputStream::current_metadata() { 
        return _impl->current_metadata(); 
    }
    
    bool InputStream::new_metadata() { return _impl->new_metadata(); }
    
    bool InputStream::good() { return _impl->good(); }
    bool InputStream::error() { return _impl->error(); }
    bool InputStream::end() { return _impl->end(); }
    void InputStream::close() { return _impl->close(); }
    bool InputStream::skip_to_next_metadata() { return _impl->skip_to_next_metadata(); }
    size_t InputStream::ByteCount() { return _impl->ByteCount(); }
    std::string InputStream::str() { return _impl->str(); }

    const std::vector<std::vector<shared<a4::io::A4Message>>>& InputStream::all_metadata() {
        return _impl->all_metadata();
    }
    const std::vector<StreamFooter>& InputStream::footers() {
        return _impl->footers();
    }
    std::vector<const google::protobuf::FileDescriptor*> InputStream::get_filedescriptors() {
        return _impl->get_filedescriptors();
    }

}
}
