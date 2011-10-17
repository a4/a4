#ifndef _RESULTS_PROCESSOR_
#define _RESULTS_PROCESSOR_

#include <a4/processor.h>
#include <a4/input_stream.h>
#include <a4/process/A4Key.pb.h>

namespace a4{ namespace process{

template<class This, class ProtoMetaData, class... Args>
shared<Storable> ResultsProcessor<This, ProtoMetaData, Args...>::_next_storable(const A4Message msg) {
    if (!msg) throw a4::Fatal("No message!"); // TODO: Should not be fatal
    if (msg.is<A4Key>()) {
        next_name = msg.as<A4Key>()->name();
        have_name = true;
        return shared<Storable>();
    }
    shared<Storable> pmsg = message_to_storable(msg);
    if (!pmsg) throw a4::Fatal("Could not convert to Storable: ", typeid(*msg.message.get()));
    if (!have_name) throw a4::Fatal("Storable without name: ", typeid(*msg.message.get()));
    have_name = false;
    return pmsg;
}


};}; // namespace

#endif
