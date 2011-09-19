#include <a4/a4process.h>
#include <a4/input_stream.h>

namespace a4{ namespace process{

const A4Message Processor::metadata_message() { 
    return _instream->current_metadata();
};

};}; // namespace
