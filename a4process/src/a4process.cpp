#include <a4/a4process.h>
#include <a4/driver.h>

namespace a4{ namespace process{

const A4Message & Processor::metadata_message() { 
    return _driver->metadata();
};

};}; // namespace
