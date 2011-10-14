#include <iostream>
#include <map>

#include <a4/register.h>

#include "A4Stream.pb.h"

namespace google{ namespace protobuf{ class Message; }}

namespace a4{ namespace io{

    internal::classreg internal::map_class_by_name(std::string name, internal::classreg reg, bool warn) {
        static std::map<std::string, internal::classreg> all_classes;
        if (reg.descriptor)
            all_classes[name] = reg;
        else 
            reg = all_classes[name];
        
        if (!reg.descriptor) {
            if (warn)
                std::cerr << "Warning, trying to get a compiled-in reader for class " << name
                          << " when there is none." << std::endl;
        }
        return reg;
    }

    template class RegisterClass<StreamHeader>;
    template class RegisterClass<StreamFooter>;
    template class RegisterClass<StartCompressedSection>;
    template class RegisterClass<EndCompressedSection>;
    template class RegisterClass<ProtoClass>;
    template class RegisterClass<TestEvent>;
    template class RegisterClass<TestMetaData>;
    template class RegisterClass<TestMergeMetaDataStatic>;

}}
