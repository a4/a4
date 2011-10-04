#include <map>

#include <a4/storable.h>

namespace a4{ namespace process{ namespace internal{
    from_message_func as_storable(int i, from_message_func func) {
        static std::map<int, from_message_func> all_storable_ids;
        if (func) all_storable_ids[i] = func;
        return all_storable_ids[i];
    }
};};};
