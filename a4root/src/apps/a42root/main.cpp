#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <TTree.h>
#include <TFile.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;

#include <a4/message.h>
#include <a4/application.h>
#include <a4/processor.h>
#include <a4/dynamic_message.h>

using a4::io::FieldContent;
using a4::io::ConstDynamicField;

#include "a4/root/RootExtension.pb.h"


class MessageTreeNode {
    
    // Keyed on the field identifier
    std::map<uint32_t, shared<MessageTreeNode>> _children;
    
    std::string _prefix;
    int _field_number;
    FieldDescriptor::CppType _field_cpp_type;
    bool _repeated;
    int _depth;
    TTree* _tree;
    TBranch* _branch;
    
public:
    
    // Root node (Whole event)
    MessageTreeNode(TTree* tree)
        : _prefix(), _field_number(-1), _field_cpp_type(FieldDescriptor::MAX_CPPTYPE),
          _repeated(false), _depth(0), _tree(tree), _branch(NULL)
    {
    
    }
    
    MessageTreeNode(const FieldDescriptor* field, uint32_t parent_depth,
                    const std::string& prefix, TTree* tree)
        : _prefix(prefix), _field_number(field->number()),
          _field_cpp_type(field->cpp_type()), _tree(tree),
          _branch(NULL)
    {
        _repeated = field->is_repeated();
        _depth = parent_depth + (_repeated ? 1 : 0);
        assert(_depth < 3);
        
        // Initialize pointers that we can point at with TTree::Bronch
        #define VALUE_TYPE(type) \
            type##_value_1_p = &type##_value_1; \
            type##_value_2_p = &type##_value_2;
        VALUE_TYPE(Bool);
        VALUE_TYPE(Int);
        VALUE_TYPE(UInt);
        VALUE_TYPE(Float);
        VALUE_TYPE(Double);
        VALUE_TYPE(Long64);
        VALUE_TYPE(ULong64);
        #undef VALUE_TYPE
    }
    
    #define VALUE_TYPE(type) \
        type##_t type##_value_0; \
        std::vector<type##_t> type##_value_1; \
        std::vector<type##_t>* type##_value_1_p; \
        std::vector<std::vector<type##_t>> type##_value_2; \
        std::vector<std::vector<type##_t>>* type##_value_2_p;
    
    VALUE_TYPE(Bool);
    VALUE_TYPE(Int);
    VALUE_TYPE(UInt);
    VALUE_TYPE(Float);
    VALUE_TYPE(Double);
    VALUE_TYPE(Long64);
    VALUE_TYPE(ULong64);
    #undef VALUE_TYPE
    
    void set(const FieldContent& content) {
        if (unlikely(!_branch))
            branch();
        #define SET_TYPE(proto_type, type) \
            case FieldDescriptor::CPPTYPE_##proto_type: \
                if (_depth == 0) \
                    type##_value_0 = content;\
                else if (_depth == 1) \
                    type##_value_1.push_back(content);\
                else if (_depth == 2) { \
                    assert(type##_value_2.size());\
                    type##_value_2.back().push_back(content);\
                } \
                break;
        switch (_field_cpp_type)  {
            SET_TYPE(INT32, Int)
            SET_TYPE(INT64, Long64)
            SET_TYPE(UINT32, UInt)
            SET_TYPE(UINT64, ULong64)
            SET_TYPE(DOUBLE, Double)
            SET_TYPE(FLOAT, Float)
            SET_TYPE(BOOL, Bool)
            default: assert(false);
            //case FieldDescriptor::CPPTYPE_ENUM: 
            //case FieldDescriptor::CPPTYPE_STRING:
            //case FieldDescriptor::CPPTYPE_MESSAGE: 
        }
        #undef SET_TYPE
    }
    
    void* get_address() {
        assert(_depth <= 2);
        #define GET_ADDRESS(proto_type, type) \
            case FieldDescriptor::CPPTYPE_##proto_type: \
                return (_depth == 0 ? reinterpret_cast<void*>(&type##_value_0) : \
                        _depth == 1 ? reinterpret_cast<void*>(&type##_value_1_p) : \
                        _depth == 2 ? reinterpret_cast<void*>(&type##_value_2_p) : NULL );
                        
        switch (_field_cpp_type) {
            GET_ADDRESS(INT32, Int)
            GET_ADDRESS(INT64, Long64)
            GET_ADDRESS(UINT32, UInt)
            GET_ADDRESS(UINT64, ULong64)
            GET_ADDRESS(DOUBLE, Double)
            GET_ADDRESS(FLOAT, Float)
            GET_ADDRESS(BOOL, Bool)
            default: assert(false);
        }
        
        #undef GET_ADDRESS
    }
    
    char root_type() {
        #define ROOT_TYPE(proto_type, type) \
            case FieldDescriptor::CPPTYPE_##proto_type: \
                return type;
                        
        switch (_field_cpp_type) {
            ROOT_TYPE(INT32, 'I')
            ROOT_TYPE(INT64, 'L')
            ROOT_TYPE(UINT32, 'i')
            ROOT_TYPE(UINT64, 'l')
            ROOT_TYPE(DOUBLE, 'D')
            ROOT_TYPE(FLOAT, 'F')
            ROOT_TYPE(BOOL, 'O')
            default: assert(false);
        }
        
        #undef ROOT_TYPE
    }
    
    std::string root_class() {
        if (_depth == 0)
            return _prefix + "/" + root_type();
        
        #define ROOT_TYPE(proto_type, type) \
            case FieldDescriptor::CPPTYPE_##proto_type: \
                if (_depth == 1) \
                    return "vector<" type ">"; \
                else if (_depth == 2) \
                    return "vector<vector<" type "> >";
                        
        switch (_field_cpp_type) {
            ROOT_TYPE(INT32, "Int_t")
            ROOT_TYPE(INT64, "Long64_t")
            ROOT_TYPE(UINT32, "UInt_t")
            ROOT_TYPE(UINT64, "ULong64_t")
            ROOT_TYPE(DOUBLE, "Double_t")
            ROOT_TYPE(FLOAT, "Float_t")
            ROOT_TYPE(BOOL, "Bool_t")
            default: assert(false);
        }
        
        #undef ROOT_TYPE
    }
    
    void branch() {
        auto rc = root_class();
        //TBranch* br = NULL;
        if (_depth == 0) {
            _branch = _tree->Branch(_prefix.c_str(), get_address(), rc.c_str());
        } else {
            //tree->Bronch(_prefix.c_str(), rc.c_str(), get_address());
            _branch = _tree->Branch(_prefix.c_str(), rc.c_str(), get_address());
        }
        
        _branch->SetBasketSize(32768);
        _branch->SetEntryOffsetLen(512);
        
        // The tree already has some entries, we need to put some into this
        // branch so that the result is correctly aligned.
        Long64_t entries = _tree->GetEntries();
        if (entries != 0)
            for (Long64_t i = 0; i < entries; i++)
                _branch->Fill();
    }
    
    // Initialize empty vectors to the correct depth
    void push_back(int push_depth) {        
        assert(push_depth == 1);
        assert(push_depth <= _depth);
        
        foreach (auto child_iter, _children)
            child_iter.second->push_back(push_depth);
        
        if (_depth == push_depth) return;
        
        #define PUSH_BACK(proto_type, type) \
            case FieldDescriptor::CPPTYPE_##proto_type: \
                type##_value_2.push_back(std::vector<type##_t>()); \
                break;
        
        switch (_field_cpp_type) {
            PUSH_BACK(INT32, Int)
            PUSH_BACK(INT64, Long64)
            PUSH_BACK(UINT32, UInt)
            PUSH_BACK(UINT64, ULong64)
            PUSH_BACK(DOUBLE, Double)
            PUSH_BACK(FLOAT, Float)
            PUSH_BACK(BOOL, Bool)
            default: assert(false);
        }
        #undef PUSH_BACK
    }

    void clear() {
        foreach (auto& child_iter, _children)
            child_iter.second->clear();
            
        if (_field_number < 0)
            return;
    
        #define CLEAR(proto_type, type) \
            case FieldDescriptor::CPPTYPE_##proto_type: \
                if (_depth == 0) \
                    type##_value_0 = -999; \
                if (_depth == 1) \
                    type##_value_1.clear(); \
                if (_depth == 2) \
                    type##_value_2.clear(); \
                break;
                        
        switch (_field_cpp_type) {
            CLEAR(INT32, Int)
            CLEAR(INT64, Long64)
            CLEAR(UINT32, UInt)
            CLEAR(UINT64, ULong64)
            CLEAR(DOUBLE, Double)
            CLEAR(FLOAT, Float)
            CLEAR(BOOL, Bool)
            default:
                // Nothing to clear.
                return;
        }
        
        #undef CLEAR
    }
    
    shared<MessageTreeNode> add_child(const FieldDescriptor* field) {
        
        std::string prefix(_prefix);
        if (field->options().HasExtension(root_prefix)) {
            prefix += field->options().GetExtension(root_prefix);
        } else if (field->options().HasExtension(root_branch)) {
            prefix += field->options().GetExtension(root_branch);
        } else {
            if (_prefix != "") prefix += "_";
            prefix += field->name();
        }
        
        auto* treenode = new MessageTreeNode(field, _depth, prefix, _tree);
        shared<MessageTreeNode> child(treenode);
        _children[field->number()] = child;
        
        return child;
    }
    
    void fill(const Message& m) {    
        std::vector<const FieldDescriptor*> fields;
        auto* refl = m.GetReflection();
        refl->ListFields(m, &fields);
                
        foreach (auto* field, fields) {
            //DEBUG("Processing field: ", field->number(), " ", field->full_name());
            
            switch (field->cpp_type()) {
            // TODO(pwaller): ENUM and STRING are not currently handled.
            case FieldDescriptor::CPPTYPE_ENUM:
            case FieldDescriptor::CPPTYPE_STRING:            
                continue;
            default:
                break;
            }
            
            auto ci = _children.find(field->number());
            shared<MessageTreeNode> child;
            
            // Check if the child exists, otherwise create it
            if (ci == _children.end()) {
                child = add_child(field);
            } else {
                child = ci->second;
            }
            
            // Create space in multi-dimensional vectors
            if (_field_number >= 0 && _repeated) {
                child->push_back(_depth);
            }
            
            ConstDynamicField fieldcontent(m, field);
            
            if (field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
                // Basic type
                if (child->_repeated) {
                    for (int i = 0; i < fieldcontent.size(); i++)
                        child->set(fieldcontent.value(i));
                } else {
                    // Skip empty fields
                    if (!fieldcontent.present())
                        return;
                    child->set(fieldcontent.value());
                }
            } else {
                // Sub-message type
                if (child->_repeated) {
                    // Repeated complex sub-message (e.g. 'repeated Electron electrons')
                    for (int i = 0; i < fieldcontent.size(); i++) {
                        child->fill(fieldcontent.submessage(i));
                    }
                } else {
                    child->fill(fieldcontent.submessage());
                }
            }
        }
    }
    
    // In the case of no children, it is a simple type, there is no substructure
    bool is_simple_type() { return _children.empty(); }
        
    void make_field(std::unordered_set<const FieldDescriptor*>& seen_fds,
                    TTree* root_tree, const FieldDescriptor* field) {
        
        bool plain_type = false;
        
        switch (field->cpp_type()) {
            case FieldDescriptor::CPPTYPE_INT32:
            case FieldDescriptor::CPPTYPE_INT64:
            case FieldDescriptor::CPPTYPE_UINT32:
            case FieldDescriptor::CPPTYPE_UINT64:
            case FieldDescriptor::CPPTYPE_DOUBLE:
            case FieldDescriptor::CPPTYPE_FLOAT:
            case FieldDescriptor::CPPTYPE_BOOL:
                plain_type = true;
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                break;
                
            case FieldDescriptor::CPPTYPE_ENUM:
            case FieldDescriptor::CPPTYPE_STRING:
                DEBUG("String encountered!", field->full_name());
                //assert(false);
            default:
                //assert(false);
                
                // NOTE: These fields are skipped for now
                return;
        };
        
        auto subnode = add_child(field);
        
        if (!plain_type) {
            subnode->make_tree(seen_fds, root_tree, field->message_type());
        }
        
    }
    
    // Note: seen_fds is copy constructed intentionally
    void make_tree(std::unordered_set<const FieldDescriptor*> seen_fds,
                   TTree* root_tree, const Descriptor* descriptor) {
        
        DEBUG("Make tree: ", descriptor->full_name());
        for (int i = 0; i < descriptor->field_count(); i++) {
            const auto* fd = descriptor->field(i);
            if (!seen_fds.insert(fd).second) {
                WARNING("Skipping recursive descriptor for the second time: ", fd->full_name());
                continue;
            }
            make_field(seen_fds, root_tree, fd);
        }
        
        const auto* pool = descriptor->file()->pool();
        std::vector<const FieldDescriptor*> extensions;
        pool->FindAllExtensions(descriptor, &extensions);
        
        foreach (const auto* extension, extensions) {
            if (!seen_fds.insert(extension).second) {
                WARNING("Skipping recursive descriptor for the second time: ", extension->full_name());
                continue;
            }
            make_field(seen_fds, root_tree, extension);
        }
    }
};

class TreeFiller {

    TTree* _root_tree;
    shared<MessageTreeNode> _top_node;

public:
    TreeFiller(const Descriptor* descriptor) {
        
        _root_tree = new TTree(descriptor->name().c_str(), 
                               descriptor->full_name().c_str());
        
        _root_tree->SetAutoFlush();
        
        _top_node.reset(new MessageTreeNode(_root_tree));
    }
    
    void fill(const a4::io::A4Message& m) {
        _top_node->clear();
        _top_node->fill(*m.message());
        _root_tree->Fill();
    }
};

class a42rootProcessor : public a4::process::Processor {
public:
    std::map<std::string, shared<TreeFiller>> class_map;
    
    TFile* f;
    
    ~a42rootProcessor() {
        class_map.clear();
        f->Write();
        f->Close();
    }

    void process_message(shared<const a4::io::A4Message> m) {
        
        auto class_name = m->message()->GetDescriptor()->full_name();
        if (class_map.find(class_name) == class_map.end()) {
            class_map[class_name].reset(new TreeFiller(m->descriptor()));
        }
        
        auto& tree_filler = *class_map[class_name];
        
        tree_filler.fill(*m);
        
        static int i = 0;
        if (i % 1000 == 0)
            INFO("Event ", i);
        i++;
    }
};


class A2RConfig : public a4::process::ConfigurationOf<a42rootProcessor> {
    public:
        std::string filename;
        int processor_count, compression_level;
        
        A2RConfig() : processor_count(0) {}

        void add_options(po::options_description_easy_init opt) {
            opt("root-file,R", po::value(&filename)->default_value("output.root"), 
                "ROOT output file");
            opt("compression-level,C", po::value(&compression_level)->default_value(1), 
                "ROOT compression level");
        }

        virtual void setup_processor(a42rootProcessor& g) {
            assert(processor_count++ == 0);
            g.f = new TFile(filename.c_str(), "RECREATE");
            g.f->SetCompressionLevel(compression_level);
        }
};

int main(int argc, const char* argv[]) {
    return a4::process::a4_main_configuration<A2RConfig>(argc, argv);
}
