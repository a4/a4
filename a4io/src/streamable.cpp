#include <map>
#include <a4/streamable.h>

from_msg_func all_class_ids(int i, from_msg_func func) {
    static std::map<int, from_msg_func> all_class_ids;
    if (func) all_class_ids[i] = func;
    return all_class_ids[i];
}
