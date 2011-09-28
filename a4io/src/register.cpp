#include <map>
#include <a4/register.h>

#include "a4/proto/io/A4Stream.pb.h"

using a4::io::internal::from_stream_func;
from_stream_func a4::io::internal::all_class_ids(int i, from_stream_func func) {
    static std::map<int, from_stream_func> all_class_ids;
    if (func) all_class_ids[i] = func;
    return all_class_ids[i];
}

namespace a4{ namespace io{

    template class RegisterClassID<A4StreamHeader>;
    template class RegisterClassID<A4StreamFooter>;
    template class RegisterClassID<A4StartCompressedSection>;
    template class RegisterClassID<A4EndCompressedSection>;
    template class RegisterClassID<A4Key>;
    template class RegisterClassID<TestEvent>;
    template class RegisterClassID<TestMetaData>;

}}
