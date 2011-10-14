#include <map>

#include <a4/storable.h>

namespace a4{ namespace process{ namespace internal{
        from_message_func as_storable(google::protobuf::Descriptor * d, from_message_func func) {
            static std::map<google::protobuf::Descriptor*, from_message_func> all_storable_ids;
            if (func) all_storable_ids[i] = func;
            return all_storable_ids[i];
        }
    };
    shared<Storable> message_to_storable(a4::io::A4Message msg) {
        return internal::as_storable(msg.descriptor)(*msg.message);
    }
};};
