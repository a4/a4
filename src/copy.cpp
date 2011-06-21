
#include "copy.h"

#include "a4/application.h"
#include "a4/results.h"
#include "a4/writer.h"
#include "pb/Event.pb.h"

void CopyProcessor::process()
{
    Event event;
    while(_reader->good() && _reader->read_event(event)) {
        _writer->write(event);
    }
}

int main(int argc, char ** argv) {
    ProcessingJobPtr pf(new CopyProcessingJob());
    return a4_main(argc, argv, pf);
}
