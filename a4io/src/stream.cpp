#include <map>
#include <a4/stream.h>

#include "a4/proto/io/A4Stream.pb.h"

using a4::io::internal::from_stream_func;
from_stream_func a4::io::internal::all_class_ids(int i, from_stream_func func) {
    static std::map<int, from_stream_func> all_class_ids;
    if (func) all_class_ids[i] = func;
    return all_class_ids[i];
}

using namespace a4::io;

template class UseClassID<A4StreamHeader>;
template class UseClassID<A4StreamFooter>;
template class UseClassID<A4StartCompressedSection>;
template class UseClassID<A4EndCompressedSection>;
template class UseClassID<A4Key>;
template class UseClassID<TestEvent>;
template class UseClassID<TestMetaData>;

