#include <iostream>
#include <map>

#include <a4/register.h>

#include "A4Stream.pb.h"

namespace google{ namespace protobuf{ class Message; }}

namespace a4{ namespace io{

    // Notice: DO NOT call other functions in the static initialization.
    // The order of initialization is NOT DETERMINED.
    internal::classreg internal::map_class(std::string name, uint32_t lookup_class_id, internal::classreg reg, bool warn) {
        static std::map<std::string, internal::classreg> all_classes;
        static std::map<uint32_t, internal::classreg> all_classes_fixed_class_id;
        if (reg.descriptor) {
            all_classes[name] = reg;
        } else if (lookup_class_id) {
            // Put the generation into the first lookup (see "static initialization order fiasco"
            if (all_classes_fixed_class_id.size() == 0) {
                // Did i mention that i miss the auto keyword?
                typedef std::pair<std::string, internal::classreg> Iter;
                foreach(Iter i, all_classes) {
                    assert(fixed_class_id.number() == kFixedClassIdFieldNumber);
                    if (i.second.descriptor->options().HasExtension(a4::fixed_class_id)) {
                        uint32_t class_id = i.second.descriptor->options().GetExtension(a4::fixed_class_id);
                        all_classes_fixed_class_id[class_id] = i.second;
                    }
                }
            }
            reg = all_classes_fixed_class_id[lookup_class_id];
        } else {
            reg = all_classes[name];
            if (!reg.descriptor) {
                if (warn)
                    std::cerr << "Warning, trying to get a compiled-in reader for class " << name
                              << " when there is none." << std::endl;
            }
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
