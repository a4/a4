#include <map>

#include <a4/storable.h>

namespace a4{ namespace process{ namespace internal{
        from_message_func as_storable(const google::protobuf::Descriptor * d, from_message_func func) {
            static std::map<const google::protobuf::Descriptor*, from_message_func> all_storable_ids;
            if (func) all_storable_ids[d] = func;
            return all_storable_ids[d];
        }
    };
    shared<Storable> message_to_storable(a4::io::A4Message msg) {
        return internal::as_storable(msg.descriptor())(*msg.message);
    }
};};
