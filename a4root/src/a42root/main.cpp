#include <map>
#include <string>

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
#include <dynamic_message.h>

#include "a4/root/RootExtension.pb.h"


class MessageTreeNode {
public:
    // Keyed on the field identifier
    std::map<uint32_t, shared<MessageTreeNode>> _children;
    
    std::string _prefix;
    const FieldDescriptor* _field;
    bool _repeated;
    int _depth;
    
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
        switch (_field->cpp_type())  {
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
                        
        switch (_field->cpp_type()) {
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
                        
        switch (_field->cpp_type()) {
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
                        
        switch (_field->cpp_type()) {
            ROOT_TYPE(INT32, "Int_t")
            ROOT_TYPE(INT64, "Long64_t")
            ROOT_TYPE(UINT32, "UInt")
            ROOT_TYPE(UINT64, "ULong64_t")
            ROOT_TYPE(DOUBLE, "Double_t")
            ROOT_TYPE(FLOAT, "Float_t")
            ROOT_TYPE(BOOL, "Bool_t")
            default: assert(false);
        }
        
        #undef ROOT_TYPE
    }
    
    void bronch(TTree* tree) {
        auto rc = root_class();
        TBranch* br = NULL;
        if (_depth == 0) {
            br = tree->Branch(_prefix.c_str(), get_address(), rc.c_str());
        } else {
            //tree->Bronch(_prefix.c_str(), rc.c_str(), get_address());
            br = tree->Branch(_prefix.c_str(), rc.c_str(), get_address());
        }
        
        br->SetBasketSize(32768);
        br->SetEntryOffsetLen(512);
    }
        
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
        
        switch (_field->cpp_type()) {
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
            
        if (not _field)
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
                        
        switch (_field->cpp_type()) {
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
    
    // Root node (Whole event)
    MessageTreeNode() : _prefix(), _field(NULL), _depth(0) {
    
    }
        
    MessageTreeNode(const FieldDescriptor* field, uint32_t parent_depth, 
                    const std::string& prefix) : _prefix(prefix), _field(field)
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
        
        shared<MessageTreeNode> child(new MessageTreeNode(field, _depth, prefix));
        _children[field->number()] = child;
        
        return child;
    }
    
    void fill(const Message& m) {
        if (_field && _repeated) {
            foreach (auto child_iter, _children)
                child_iter.second->push_back(_depth);
        }
                
        if (is_simple_type()) {
            // Simple types (numeric, etc)
            ConstDynamicField fieldcontent(m, _field);
            if (_repeated) {
                for (int i = 0; i < fieldcontent.size(); i++)
                    set(fieldcontent.value(i));
            } else {
                set(fieldcontent.value());
            }
        } else {
            // 
            foreach (auto child_iter, _children) {
                auto child = child_iter.second;
                ConstDynamicField fieldcontent(m, child->_field);
                if (not child->is_simple_type()) {
                    if (child->_repeated) {
                        // Repeated complex sub-message (e.g. 'repeated Electron electrons')
                        for (int i = 0; i < fieldcontent.size(); i++) {
                            child->fill(fieldcontent.submessage(i));
                        }
                    } else {
                        child->fill(fieldcontent.submessage());
                    }
                } else {
                    child->fill(m);
                }
            }
        }
    }
    
    // In the case of no children, it is a simple type, there is no substructure
    bool is_simple_type() { return _children.empty(); }
};

class TreeFiller {

    TTree* _root_tree;
    shared<MessageTreeNode> top_node;

public:
    TreeFiller(const Descriptor* descriptor) 
        : top_node(new MessageTreeNode) 
    {
        
        _root_tree = new TTree(descriptor->name().c_str(), 
                               descriptor->full_name().c_str());
        
        _root_tree->SetAutoFlush();
        
        make_tree(top_node, _root_tree, descriptor);
        
    }
    
    void make_tree(shared<MessageTreeNode> node, TTree* root_tree, 
                   const Descriptor* descriptor) {
        for (int i = 0; i < descriptor->field_count(); i++) {
            auto* field = descriptor->field(i);
            
            auto subnode = node->add_child(field);
            
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
                    assert(false);
                default:
                    assert(false);
            };
            
            if (plain_type) {
                subnode->bronch(root_tree);
            } else {
                make_tree(subnode, root_tree, field->message_type());
            }
            
        }
    }
    
    void fill(const a4::io::A4Message& m) {
        top_node->clear();
        top_node->fill(*m.message());
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
        int processor_count;
        
        A2RConfig() : processor_count(0) {};


        void add_options(po::options_description_easy_init opt) {
            opt("root-file,R", po::value(&filename)->default_value("output.root"), "ROOT output file");
        }

        virtual void setup_processor(a42rootProcessor& g) {
            assert(processor_count++ == 0);
            g.f = new TFile(filename.c_str(), "RECREATE");
        }
};

int main(int argc, const char* argv[]) {
    return a4::process::a4_main_configuration<A2RConfig>(argc, argv);
}
