
#include "copy.h"

#include "a4/application.h"
#include "a4/results.h"
#include "a4/writer.h"
#include "pb/Event.pb.h"

void CopyProcessor::process()
{
    Event event;
    Writer w("copy.a4", "Event", Event::kCLASSIDFieldNumber);
    while(_reader->good() && _reader->read_event(event)) {
        w.write(event);
    }
}

int main(int argc, char ** argv) {
    ProcessorFactoryPtr pf(new CopyProcessorFactory());
    ResultsPtr r;
    return a4_main(argc, argv, pf, r);
    //r->print();
}
