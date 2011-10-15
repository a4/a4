#include "a4/input_stream.h"
#include "input_stream_impl.h"

namespace a4{ namespace io{

    // Forward everything to the implementation object.
    InputStream::InputStream(std::string url) {
        _impl.reset(new InputStreamImpl(resource_from_url(url), url));
    };

    InputStream::InputStream(unique<InputStreamImpl> impl) {
        _impl = std::move(impl);
    }
    InputStream::~InputStream() {}
    A4Message InputStream::next() { return _impl->next(); }
    A4Message InputStream::next_with_metadata() {
        return _impl->next_with_metadata();
    }
    const A4Message InputStream::current_metadata() { 
        return _impl->current_metadata(); 
    }
    bool InputStream::new_metadata() {return _impl->new_metadata(); }
    bool InputStream::good() { return _impl->good(); }
    bool InputStream::error() { return _impl->error(); }
    bool InputStream::end() { return _impl->end(); }
    std::string InputStream::str() { return _impl->str(); }

};};
