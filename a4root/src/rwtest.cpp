#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout; using std::endl; using std::cerr;

#include <boost/function.hpp>
#include <boost/bind.hpp>
//using namespace boost;
using boost::bind;
using boost::function;

#include <a4/output.h>
#include <a4/input.h>

#include <TBranch.h>
#include <TBranchElement.h>
#include <TChain.h>
#include <TLeaf.h>
#include <TVirtualCollectionProxy.h>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
using google::protobuf::Message;
using google::protobuf::MessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Reflection;

#include "Event.pb.h"

//using namespace std;
using namespace a4::io;
using namespace a4::example::root;

using namespace google;

template<typename T>
class FieldSetter
{
public:
    typedef function<void (const Reflection*, Message*, const FieldDescriptor*, T)> type;
};

typedef function<shared<Message> ()>        RootToMessageFactory;
typedef function<void (Message*)>           Copier;
typedef vector<Copier>                      Copiers;
typedef function<void (Message*, size_t)> CopierN;
typedef vector<CopierN>                     CopierNs;

A4RegisterClass(Event);
A4RegisterClass(Metadata);

const vector<const FieldDescriptor*> get_fields(const Descriptor* d) {
    vector<const FieldDescriptor*> result;
    for (int i = 0; i < d->field_count(); i++)
        result.push_back(d->field(i));
    return result;
}

template<typename T>
void call_setter(
    function<void (T, Message*)> setter, Message* message, void* value)
{
    //cout << "Setter called! " << *reinterpret_cast<T*>(value) << endl;
    setter(*reinterpret_cast<T*>(value), message);
}

template<typename T> typename FieldSetter<T>::type reflection_setter() { throw "Don't use this one"; };
#define DEFINE_SETTER(T, SetterName) \
    template<> FieldSetter<T>::type reflection_setter<T>() { \
        return bind(&::google::protobuf::Reflection::SetterName, _1, _2, _3, _4); \
    }
    
DEFINE_SETTER(int32_t,   SetInt32);
DEFINE_SETTER(int64_t,   SetInt64);
DEFINE_SETTER(uint32_t,  SetUInt32);
DEFINE_SETTER(uint64_t,  SetUInt64);
DEFINE_SETTER(float,    SetFloat);
DEFINE_SETTER(double,   SetDouble);
DEFINE_SETTER(bool,     SetBool);
// TODO: SetString
//        SetEnum


template<typename T>
Copier generic_make_field_setter(void* p, const FieldDescriptor* f, const Reflection*& r)
{
    // This horrifying beastie returns a `F`=[function which takes a Message*].
    // `F` fills one field (`f`) of the message with the contents at `p`.
    typename FieldSetter<T>::type test = reflection_setter<T>();
    function<void (T, Message*)> set_reflection_field = bind(test, r, _2, f, _1);
    function<void (function<void (T, Message*)>, Message*, void*)> set_pointer_inside_message = call_setter<T>;
    return bind(set_pointer_inside_message, set_reflection_field, _1, p);
}

Copier copier_from_field_descriptor(void* p, const FieldDescriptor* f, const Reflection*& r)
{
    switch (f->cpp_type())
    {
        case FieldDescriptor::CPPTYPE_BOOL:    return generic_make_field_setter<bool>(p, f, r);
        
        case FieldDescriptor::CPPTYPE_INT32:   return generic_make_field_setter<int32_t>(p, f, r);
        case FieldDescriptor::CPPTYPE_INT64:   return generic_make_field_setter<int64_t>(p, f, r);
        case FieldDescriptor::CPPTYPE_UINT32:  return generic_make_field_setter<uint32_t>(p, f, r);
        case FieldDescriptor::CPPTYPE_UINT64:  return generic_make_field_setter<uint64_t>(p, f, r);
        
        case FieldDescriptor::CPPTYPE_DOUBLE:  return generic_make_field_setter<double>(p, f, r);
        case FieldDescriptor::CPPTYPE_FLOAT:   return generic_make_field_setter<float>(p, f, r);
            
        default:
            cerr << "Unknown field type " << f->type() << endl;
            throw "Aaaaaaargh!";
    }
}

shared<Message> message_factory(const Message* default_instance, const Copiers& copiers)
{
    shared<Message> message(default_instance->New());
    
    for (auto copier: copiers) copier(message.get());
    
    return message;
}

tempate<typename T>
void set_messages(Message** message, vector<T>* input, )
{
    typename FieldSetter<T>::type test = reflection_setter<T>();
    function<void (T, Message*)> set_reflection_field = bind(test, r, _2, f, _1);
    
    size_t i = 0;
    for (auto value: input)
        call_setter<T>(message[i++], value);
}

void submessage_factory(
    Message* parent,
    const Reflection* parent_refl,
    const FieldDescriptor* field_desc,
    const CopierNs& copiers,
    function<size_t ()> compute_count
    )
{
    size_t count = 0; //compute_count();
    Message** submessages = new Messages*[count];
    for (size_t i = 0; i < count; i++)
        submessages[i] = parent_refl->AddMessage(parent, field_desc);
    
    for (auto submessage_setter: submessage_setters)
        submessage_setter(submessages);
}



Copier make_submessage_factory(TTree& tree, const Reflection* parent_refl, const FieldDescriptor* field_desc, const std::string& prefix="")
{
    const Descriptor* desc = field_desc->message_type();
    assert(desc);
    const Message* default_instance = MessageFactory::generated_factory()->GetPrototype(desc);
    const Reflection* refl = default_instance->GetReflection();
    
    //vector<pair<TBranchElement*, FieldDescriptor*>> 
    
    foreach (auto field, get_fields(desc)) {
        if (!field->options().HasExtension(root_branch))
            continue;
        auto leafname = prefix + field->options().GetExtension(root_branch);
        TBranchElement* br = dynamic_cast<TBranchElement*>(tree.GetBranch(leafname.c_str()));
        // Assume for the moment std::vector type
        // Idea: need to build one submessage_setter per branchelement
        auto arr = reinterpret_cast<vector<float>*>(br->GetObject());
        cout << "Trying " << leafname << " " << br->GetClassName() << " - " << arr->size() << endl;
        assert(br);
    }
    function<size_t ()> compute_count;
    //bind(&TVirtualCollectionProxy::Size, counter_proxy);
    
    CopierNs copiers;
    Copier copier = bind(submessage_factory, _1, parent_refl, field_desc, copiers, compute_count);
    return copier;
}

RootToMessageFactory make_message_factory(TTree& tree, const Descriptor* desc, const std::string& prefix="")
{
    Copiers copiers;
    
    const Message* default_instance = MessageFactory::generated_factory()->GetPrototype(desc);
    const Reflection* refl = default_instance->GetReflection();
    
    assert(!desc->nested_type_count()); // unsupported
    
    foreach (auto field, get_fields(desc)) {
        if (field->is_repeated()) {
            if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                //cout << "Repeatede message type!" << endl;
                //field->message_type();
                if (field->options().HasExtension(root_prefix)) {
                    auto this_prefix = prefix + field->options().GetExtension(root_prefix);
                    copiers.push_back(make_submessage_factory(tree, refl, field, this_prefix));
                }
            } else {
                // Don't know what to do with these yet!
                assert(false);
            }
        
        } else {
        
            if (field->options().HasExtension(root_branch)) {
                auto leafname = prefix + field->options().GetExtension(root_branch);
                TLeaf* b = tree.GetLeaf(leafname.c_str());
                
                cout << "Copying leaf: " << b->GetName() << endl;
                copiers.push_back(copier_from_field_descriptor(b->GetValuePointer(), field, refl));
                
            } else if (field->options().HasExtension(root_prefix)) {
                assert(false); // Don't specify both a root_branch _and_ root_prefix.
            } 
        }
    }
    
    return bind(message_factory, default_instance, copiers);
}

void copy_tree(TTree& tree, shared<A4OutputStream> stream, Long64_t entries = -1)
{
    if (entries < 0)
        entries = tree.GetEntries();
    
    tree.GetEntry(0); // Get one entry to make sure the branches are setup
    RootToMessageFactory event_factory = make_message_factory(tree, Event::descriptor());
    
    for (Long64_t i = 0; i < entries; i++)
    {    
        tree.GetEntry(i);
        
        // TODO: Deal with branch address changes?
        
        shared<Event> event = dynamic_shared_cast<Event>(event_factory());
        stream->write(*event);
    }
    
    Metadata m;
    m.set_total_events(entries);
    stream->metadata(m);
}

int main(int argc, char ** argv) {
    A4Output a4o("test_io.a4", "Event");

    shared<A4OutputStream> stream = a4o.get_stream(); 
    stream->content_cls<Event>();
    stream->metadata_cls<Metadata>();

    TChain input("photon");
    input.Add("input/*.root*");

    copy_tree(input, stream, 5);
}


