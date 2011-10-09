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
#include "Metadata.pb.h"

//using namespace std;
using namespace a4::io;
using namespace a4::example::root;

using namespace google;

template<typename T>
class Setter
{
public:
    typedef function<void (const Reflection*, Message*, const FieldDescriptor*, T)> type;
    typedef function<void (Message*, T)> msg;
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
    typename Setter<T>::msg setter, Message* message, void* value)
{
    //cout << "Setter called! " << *reinterpret_cast<T*>(value) << endl;
    setter(message, *reinterpret_cast<T*>(value));
}

template<typename T> typename Setter<T>::type reflection_setter() { throw "Don't use this one"; };
#define DEFINE_SETTER(T, SetterName) \
    template<> Setter<T>::type reflection_setter<T>() { \
        return bind(&::google::protobuf::Reflection::SetterName, _1, _2, _3, _4); \
    }
    
DEFINE_SETTER(int32_t,   SetInt32);
DEFINE_SETTER(int64_t,   SetInt64);
DEFINE_SETTER(uint32_t,  SetUInt32);
DEFINE_SETTER(uint64_t,  SetUInt64);
DEFINE_SETTER(float,    SetFloat);
DEFINE_SETTER(double,   SetDouble);
DEFINE_SETTER(bool,     SetBool);
DEFINE_SETTER(std::string, SetString);
// TODO: SetString
//        SetEnum

template<typename T>
typename Setter<T>::msg make_setter(const FieldDescriptor* f, const Reflection* r)
{
    typename Setter<T>::type test = reflection_setter<T>();
    typename Setter<T>::msg set_reflection_field = bind(test, r, _1, f, _2);
    return set_reflection_field;
}

template<typename T>
Copier generic_make_field_setter(void* p, const FieldDescriptor* f, const Reflection* r)
{
    // This horrifying beastie returns a `F`=[function which takes a Message*].
    // `F` fills one field (`f`) of the message with the contents at `p`.
    typename Setter<T>::msg set_reflection_field = make_setter<T>(f, r);
    function<void (typename Setter<T>::msg, Message*, void*)> set_pointer_inside_message = call_setter<T>;
    return bind(set_pointer_inside_message, set_reflection_field, _1, p);
}

Copier copier_from_field_descriptor(void* p, const FieldDescriptor* f, const Reflection*& r)
{
    #define FIELD(cpptype, T) \
        case FieldDescriptor::cpptype: return generic_make_field_setter<T>(p, f, r);
            
    switch (f->cpp_type())
    {
        FIELD(CPPTYPE_BOOL,   bool);
        
        FIELD(CPPTYPE_INT32,  int32_t);
        FIELD(CPPTYPE_INT64,  int64_t);
        FIELD(CPPTYPE_UINT32, uint32_t);
        FIELD(CPPTYPE_UINT64, uint64_t);
        
        FIELD(CPPTYPE_FLOAT,  float);
        FIELD(CPPTYPE_DOUBLE, double);
        
        FIELD(CPPTYPE_STRING, std::string);
        
        default:
            cerr << "Unknown field type in copier_from_field_descriptor" << f->cpp_type() << endl;
            throw "Aaaaaaargh!";
    }
            
    #undef FIELD
}

shared<Message> message_factory(const Message* default_instance, const Copiers& copiers)
{
    shared<Message> message(default_instance->New());
    
    foreach (const Copier& copier, copiers) copier(message.get());
    
    return message;
}

template<typename T>
void set_messages(Message** message, vector<T>* input)
{
    // typename FieldSetter<T>::type test = reflection_setter<T>();
    // function<void (T, Message*)> set_reflection_field = bind(test, r, _2, f, _1);
    
    size_t i = 0;
    foreach (const T& value, input)
        call_setter<T>(message[i++], value);
}

typedef Message* MessageP;

typedef function<void (Message**, size_t)> SubmessageSetter;
typedef vector<SubmessageSetter> SubmessageSetters;

void submessage_factory(
    Message* parent,
    const Reflection* parent_refl,
    const FieldDescriptor* field_desc,
    const SubmessageSetters& submessage_setters,
    function<size_t ()> compute_count
    )
{
    size_t count = compute_count();
    
    MessageP* submessages = new MessageP[count];
    
    for (size_t i = 0; i < count; i++)
    {
        submessages[i] = parent_refl->AddMessage(parent, field_desc);
        //submessages[i]->GetReflection()->SetFloat(submessages[i], submessages[i]->GetDescriptor()->FindFieldByName("eta_s2"), 5.);
    }
    
    foreach (const SubmessageSetter& submessage_setter, submessage_setters) 
        submessage_setter(submessages, count);
        
    delete [] submessages;
}

template<typename T>
void submessage_setter(Message** messages, size_t count, TBranchElement* br, const typename Setter<T>::msg& setter)
{
    vector<T>* values = reinterpret_cast<vector<T>* >(br->GetObject());
    for (size_t i = 0; i < count; i++)
        setter(messages[i], (*values)[i]);
}

template<typename T>
size_t count_getter(TBranchElement* br)
{
    return reinterpret_cast<vector<T>* >(br->GetObject())->size();
}

function<size_t ()> make_count_getter(TBranchElement* br, const FieldDescriptor* field)
{
    #define FIELD(cpptype, T) \
        case FieldDescriptor::cpptype: \
            return bind(count_getter<T>, br)
    
    switch (field->cpp_type())
    {
        FIELD(CPPTYPE_BOOL,   bool);
        
        FIELD(CPPTYPE_INT32,  int32_t);
        FIELD(CPPTYPE_INT64,  int64_t);
        FIELD(CPPTYPE_UINT32, uint32_t);
        FIELD(CPPTYPE_UINT64, uint64_t);
        
        FIELD(CPPTYPE_FLOAT,  float);
        FIELD(CPPTYPE_DOUBLE, double);
        
        FIELD(CPPTYPE_STRING, std::string);
        
        default:
            cerr << "Unknown field type in counter " << field->cpp_type() << endl;
            throw "Aaaaaaargh!";
    }
            
    #undef FIELD
}

SubmessageSetter make_submessage_setter(TBranchElement* br, const FieldDescriptor* field, const Reflection* refl)
{
    #define FIELD(cpptype, T) \
        case FieldDescriptor::cpptype: \
            return bind(submessage_setter<T>, _1, _2, br, make_setter<T>(field, refl))
    
    switch (field->cpp_type())
    {
        FIELD(CPPTYPE_BOOL,   bool);
        
        FIELD(CPPTYPE_INT32,  int32_t);
        FIELD(CPPTYPE_INT64,  int64_t);
        FIELD(CPPTYPE_UINT32, uint32_t);
        FIELD(CPPTYPE_UINT64, uint64_t);
        
        FIELD(CPPTYPE_FLOAT,  float);
        FIELD(CPPTYPE_DOUBLE, double);
        
        FIELD(CPPTYPE_STRING, std::string);
        
        default:
            cerr << "Unknown field type in submsg setter " << field->cpp_type() << endl;
            throw "Aaaaaaargh!";
    }
    #undef FIELD
}

void null_copier(Message*) {}

Copier make_submessage_factory(TTree& tree, const Reflection* parent_refl, const FieldDescriptor* field_desc, const std::string& prefix="")
{
    SubmessageSetters submessage_setters;
    const Descriptor* desc = field_desc->message_type();
    assert(desc);
    const Message* default_instance = MessageFactory::generated_factory()->GetPrototype(desc);
    const Reflection* refl = default_instance->GetReflection();
    
    //vector<pair<TBranchElement*, FieldDescriptor*>> 
    function<size_t ()> compute_count;
    
    foreach (auto field, get_fields(desc)) {
        if (!field->options().HasExtension(root_branch))
            continue; // Not a root branch
        
        if (field->is_repeated())
            continue; // TODO(pwaller): it's a vector<vector<...
            
        auto leafname = prefix + field->options().GetExtension(root_branch);
        TBranchElement* br = dynamic_cast<TBranchElement*>(tree.GetBranch(leafname.c_str()));
        if (!br) {
            cout << "Branch " << leafname << " didn't load" << " - " << prefix << "-" << field->options().GetExtension(root_branch) << endl;
            continue;
        }

        // Re-enable this branch
        br->ResetBit(kDoNotProcess);
        
        if (!compute_count)
            compute_count = make_count_getter(br, field);
        
        //cout << "Pushing back!" << endl;
        submessage_setters.push_back(make_submessage_setter(br, field, refl));
        // Assume for the moment std::vector type
        // Idea: need to build one submessage_setter per branchelement
        auto arr = reinterpret_cast<vector<float>*>(br->GetObject());
        //cout << "Trying " << leafname << " " << br->GetClassName() << " - " << arr->size() << endl;
        assert(br);
    }
    
    if (!compute_count) {
        std::cerr << "Couldn't find a suitable field to compute count " << field_desc->full_name() << std::endl;
        return null_copier;
    }
    // If we don't have that then we can't do anything.
    assert(compute_count);
    
    Copier copier = bind(submessage_factory, _1, parent_refl, field_desc, submessage_setters, compute_count);
    return copier;
}

RootToMessageFactory make_message_factory(TTree& tree, const Descriptor* desc, const std::string& prefix="")
{
    Copiers copiers;
    
    const Message* default_instance = MessageFactory::generated_factory()->GetPrototype(desc);
    assert(default_instance);
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
                // Enable this branch
                tree.GetBranch(leafname.c_str())->ResetBit(kDoNotProcess);
                //cout << "Copying leaf: " << b->GetName() << endl;
                copiers.push_back(copier_from_field_descriptor(b->GetValuePointer(), field, refl));
                
            } else if (field->options().HasExtension(root_prefix)) {
                assert(false); // Don't specify both a root_branch _and_ root_prefix.
            } 
        }
    }
    
    return bind(message_factory, default_instance, copiers);
}

class EventFactoryBuilder : public TObject
{
TTree& _tree;
const Descriptor* _descriptor;
RootToMessageFactory* _factory;

public:

    EventFactoryBuilder(TTree& t, const Descriptor* d, RootToMessageFactory* f) :
        _tree(t), _descriptor(d), _factory(f)
    {}

    Bool_t Notify() 
    { 
        assert(_descriptor);
        //assert(MessageFactory::generated_factory()->GetPrototype(_descriptor));
        cout << "Notify start" << endl;
        (*_factory) = make_message_factory(_tree, _descriptor);
        cout << "Notify finish" << endl;
    }
};

void copy_tree(TTree& tree, shared<A4OutputStream> stream, Long64_t entries = -1)
{
    Long64_t tree_entries = tree.GetEntries();
    if (entries > tree_entries)
        entries = tree_entries;
    if (entries < 0)
        entries = tree_entries;
    if (!entries)
        return;
        
    cout << "Will process " << entries << " entries" << endl;
    
    // Nothing to do!
    if (!entries)
        return;
    
    tree.GetEntry(0); // Get one entry to make sure the branches are setup
    // Disable all branches to be enabled when we make the message factory
    tree.SetBranchStatus("*", false);
    
    RootToMessageFactory event_factory;
    EventFactoryBuilder builder(tree, Event::descriptor(), &event_factory);
    
    tree.SetNotify(&builder);
    builder.Notify(); // Make the first event factory type
    
    size_t total_bytes_read = 0;
    
    for (Long64_t i = 0; i < entries; i++)
    {
        size_t read_data = tree.GetEntry(i);
        total_bytes_read += read_data;
        if (i % 100 == 0)
            cout << "Progress " << i << " / " << entries << " (" << read_data << ")" << endl;
        
        shared<Event> event = dynamic_shared_cast<Event>(event_factory());
        stream->write(*event);
    }
    
    //Metadata m;
    //m.set_total_events(entries);
    //stream->metadata(m);
    
    cout << "Copied " << entries << " entries (" << total_bytes_read << ")" << endl;
}

int main(int argc, char ** argv) {
    A4Output a4o("test_io.a4", "Event");

    shared<A4OutputStream> stream = a4o.get_stream(); 
    stream->content_cls<Event>();
    stream->metadata_cls<Metadata>();

    TChain input("photon");
    input.Add("input/*.root*");

    copy_tree(input, stream, 2000);
}


