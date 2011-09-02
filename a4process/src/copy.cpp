#include "copy.h"

#include "a4/application.h"
#include "a4/writer.h"
#include "pb/Event.pb.h"

void CopyProcessor::process_event(Event &event)
{
    write_event(event);
}

int main(int argc, char ** argv) {
    CopyProcessingJob j;
    return a4_main(argc, argv, j);
}
