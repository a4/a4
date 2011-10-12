#include <iostream>
#include <map>

#include <a4/register.h>

#include "a4/proto/io/A4Stream.pb.h"

namespace google{ namespace protobuf{ class Message; }}

shared<google::protobuf::Message> a4::io::internal::bad_from_stream_func(google::protobuf::io::CodedInputStream *) { assert(false); }

using a4::io::internal::from_stream_func;
from_stream_func a4::io::internal::all_class_ids(int i, from_stream_func func, bool warn) {
    static std::map<int, from_stream_func> all_class_ids;
    if (func) 
        all_class_ids[i] = func;
    else 
        func = all_class_ids[i];
    
    if (!func) {
        if (warn)
            std::cerr << "Warning, trying to get a reader for CLASS_ID " << i 
                      << " when there is none." << std::endl;
        func = &bad_from_stream_func;
    }
    return func;
}

namespace a4{ namespace io{

    template class RegisterClassID<A4StreamHeader>;
    template class RegisterClassID<A4StreamFooter>;
    template class RegisterClassID<A4StartCompressedSection>;
    template class RegisterClassID<A4EndCompressedSection>;
    template class RegisterClassID<A4Proto>;
    template class RegisterClassID<TestEvent>;
    template class RegisterClassID<TestMetaData>;
    template class RegisterClassID<TestMergeMetaDataNoFields>;

}}
