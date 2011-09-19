
#include <a4/message.h>

namespace a4{ namespace process{

class Driver {
    public:
        virtual a4::io::A4Message & metadata() = 0;
};


};}; // namespace a4::process
