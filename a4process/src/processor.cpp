#include <a4/processor.h>

#include <google/protobuf/message.h>

#include <a4/message.h>

#include <a4/input_stream.h>
#include <a4/results_processor.h>

#include <a4/process/A4Key.pb.h>

namespace a4 {
namespace process {


void OutputAdaptor::write(const google::protobuf::Message& m) {
    write(shared<const A4Message>(new A4Message(m)));
}

void OutputAdaptor::metadata(const google::protobuf::Message& m) {
    metadata(shared<const A4Message>(new A4Message(m)));
}

}
}
