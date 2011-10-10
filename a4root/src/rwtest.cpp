#include <iostream>
#include <string>
#include <vector>

//using std::string;
using std::vector;
//using std::cout; using std::endl; using std::cerr;

#include <boost/function.hpp>
#include <boost/bind.hpp>
//using namespace boost;
using boost::bind;
using boost::function;

#include <a4/output.h>
#include <a4/input.h>
#include <a4/string.h>

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
    setter(message, *reinterpret_cast<T*>(value));
}

template<typename T> typename Setter<T>::type reflection_setter() { 
    throw std::runtime_error(str_cat("Unknown type: ", typeid(T), ", add a DEFINE_SETTER line in ", __FILE__)); 
};
#define DEFINE_SETTER(T, SetterName) \
    template<> Setter<T>::type reflection_setter<T>() { \
        return bind(&::google::protobuf::Reflection::SetterName, _1, _2, _3, _4); \
    }
    
DEFINE_SETTER(int32_t,   SetInt32);
DEFINE_SETTER(short,    SetInt32);
DEFINE_SETTER(int64_t,   SetInt64);
DEFINE_SETTER(uint32_t,  SetUInt32);
DEFINE_SETTER(unsigned short, SetUInt32);
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
            throw std::runtime_error(str_cat("Unknown field type in copier_from_field_descriptor ", f->cpp_type()));
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
        setter(messages[i], values->at(i));
}

/// Returns the current size of the `branch_element` which represents a type `T` 
/// in a ROOT TTree.
template<typename T>
size_t count_getter(TBranchElement* branch_element)
{
    void* pointer = branch_element->GetObject();
    assert(pointer);
    return reinterpret_cast<vector<T>* >(pointer)->size();
}

/// Returns a function which knows the current length of `branch_element`.
function<size_t ()> make_count_getter(TBranchElement* branch_element, 
    const FieldDescriptor* field)
{
    #define FIELD(cpptype, T) \
        case FieldDescriptor::cpptype: \
            return bind(count_getter<T>, branch_element)
    
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
            throw std::runtime_error(str_cat("Unknown field type in counter ", field->cpp_type()));
    }
            
    #undef FIELD
}

/// Returns a function which will set one `field` of a protobuf 
// "repeated Message" from the branch `branch_element`.
SubmessageSetter make_submessage_setter(TBranchElement* br, 
    const FieldDescriptor* field, const Reflection* refl)
{
    // Return a function which will copy elements from `branch_element` into the
    // `field` of message `_1`, assuming it is a vector<T> of length `_2`.
    // This uses the `refl` argument.
    #define BIND(T) \
        return bind(submessage_setter<T>, _1, _2, br, make_setter<T>(field, refl))
    #define FIELD(cpptype, T) \
        case FieldDescriptor::cpptype: \
            BIND(T)
    
    switch (field->cpp_type())
    {
        FIELD(CPPTYPE_BOOL,   bool);
        
        case FieldDescriptor::CPPTYPE_INT32:
            if (br->GetTypeName() == std::string("vector<int>"))
                BIND(int32_t);
            else if (br->GetTypeName() == std::string("vector<long>"))
                BIND(int32_t);
            else if (br->GetTypeName() == std::string("vector<char>"))
                BIND(char);
            else if (br->GetTypeName() == std::string("vector<short>"))
                BIND(short);
            else
                throw std::runtime_error(std::string("Unknown type identifier on root file: ") + br->GetTypeName());
        
        case FieldDescriptor::CPPTYPE_UINT32:
            if (br->GetTypeName() == std::string("vector<unsigned int>"))
                BIND(uint32_t);
            else if (br->GetTypeName() == std::string("vector<unsigned long>"))
                BIND(uint32_t);
            else if (br->GetTypeName() == std::string("vector<unsigned char>"))
                BIND(unsigned char);
            else if (br->GetTypeName() == std::string("vector<unsigned short>"))
                BIND(unsigned short);
            else
                throw std::runtime_error(std::string("Unknown type identifier on root file: ") + br->GetTypeName());
        
        FIELD(CPPTYPE_INT64,  int64_t);
        FIELD(CPPTYPE_UINT64, uint64_t);
        
        FIELD(CPPTYPE_FLOAT,  float);
        FIELD(CPPTYPE_DOUBLE, double);
        
        FIELD(CPPTYPE_STRING, std::string);
        
        default:
            throw std::runtime_error(str_cat("Unknown field type in submsg setter ", field->cpp_type()));
    }
    #undef FIELD
    #undef BIND
}

/// Used to indicate that that it is not possible to copy this message.
/// Gives an assertion failure if called.
void null_copier(Message*) { assert(false); }

Copier make_submessage_factory(TTree& tree, const Reflection* parent_refl, const FieldDescriptor* parent_field, const std::string& prefix="")
{
    SubmessageSetters submessage_setters;
    const Descriptor* desc = parent_field->message_type();
    assert(desc);
    const Message* default_instance = MessageFactory::generated_factory()->GetPrototype(desc);
    const Reflection* refl = default_instance->GetReflection();
    
    function<size_t ()> compute_count;
    
    foreach (const FieldDescriptor* field, get_fields(desc)) {
        if (!field->options().HasExtension(root_branch))
        {
            const std::string warning = str_cat(field->full_name(), 
                " has no conversion specifier, e.g. [(root_prefix=\"", 
                field->name(), "\")];.");
            std::cerr << warning << std::endl;
            continue;
            //throw std::runtime_error(warning);
        }
        
        const std::string leafname = prefix + field->options().GetExtension(root_branch);
        
        TBranchElement* br = dynamic_cast<TBranchElement*>(tree.GetBranch(leafname.c_str()));
        if (!br) {
            std::cerr << "WARNING: " << "[" << parent_field->full_name() << " / "
                      << field->full_name() << "]"
                      << " couldn't load ROOT branch " << leafname << std::endl;
            continue;
        }
        
        if (field->is_repeated())
        {
            throw std::runtime_error(str_cat("Can't currently convert ", 
                field->full_name(), " with ROOT branch ", leafname, " type ",
                br->GetTypeName()));
            continue; // TODO(pwaller): it's a vector<vector<...
        }            

        // Re-enable this branch
        br->ResetBit(kDoNotProcess);
        
        // Do this exactly once over the loop over get_fields(desc).
        if (!compute_count)
            compute_count = make_count_getter(br, field);
        
        // TODO(pwaller): At the moment, this assumes that br is a vector< type.
        // At some point it might be necessary to extend this to allow for
        // other classes.
        submessage_setters.push_back(make_submessage_setter(br, field, refl));
    }
    
    if (!compute_count) {
        std::cerr << "WARNING: " << parent_field->full_name() 
                  << " No suitable field found to compute length!" << std::endl;
        return null_copier;
    }
    
    Copier copier = bind(submessage_factory, _1, parent_refl, parent_field, submessage_setters, compute_count);
    return copier;
}

RootToMessageFactory make_message_factory(TTree& tree, const Descriptor* desc, const std::string& prefix="")
{
    Copiers copiers;
    
    const Message* default_instance = MessageFactory::generated_factory()->GetPrototype(desc);
    assert(default_instance);
    const Reflection* refl = default_instance->GetReflection();
    
    // TODO(pwaller): I don't know what are the implications of a non-zero NTC at the moment.
    assert(!desc->nested_type_count());
    
    foreach (auto field, get_fields(desc)) {
        if (field->is_repeated()) {
            if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                if (field->options().HasExtension(root_prefix)) {
                    const std::string this_prefix = prefix + field->options().GetExtension(root_prefix);
                    copiers.push_back(make_submessage_factory(tree, refl, field, this_prefix));
                } else {
                    const std::string warning = str_cat(field->full_name(), 
                        " has no conversion specifier, e.g. [(root_prefix=\"", 
                        field->name(), "\")];.");
                    std::cerr << warning << std::endl;
                    continue;
                    //throw std::runtime_error(warning);
                }
            } else {
                // Don't know what to do with these yet!
                // These are non-message repeated fields on the Event message class.
                assert(false);
            }
        
        } else {
        
            if (field->options().HasExtension(root_branch)) {
                auto leafname = prefix + field->options().GetExtension(root_branch);
                TLeaf* b = tree.GetLeaf(leafname.c_str());
                if (!b)
                {
                    std::cerr << "Branch specified in protobuf file but not in TTree: " << leafname << std::endl;
                    continue;
                }
                
                // Enable this branch
                tree.GetBranch(leafname.c_str())->ResetBit(kDoNotProcess);
                
                copiers.push_back(copier_from_field_descriptor(b->GetValuePointer(), field, refl));
                
            } else if (field->options().HasExtension(root_prefix)) {
                const std::string prefix = field->options().GetExtension(root_prefix);
                throw std::runtime_error(str_cat(field->full_name(), 
                    " is not repeated but has a [(root_prefix=\"", prefix, "\")]. "
                    "These are not compatible with one-another."));
            } else {
                // What to do here? Warn the user that we're ignoring the field?
                const std::string warning = str_cat(field->full_name(), 
                    " has no conversion specifier, e.g. [(root_branch=\"", 
                    field->name(), "\")];.");
                std::cerr << warning << std::endl;
                continue;
                //throw std::runtime_error(warning);
            }
        }
    }
    
    return bind(message_factory, default_instance, copiers);
}

/// Builds a RootToMessageFactory when Notify() is called.
class EventFactoryBuilder : public TObject
{
TTree& _tree;
const Descriptor* _descriptor;
RootToMessageFactory* _factory;

public:

    EventFactoryBuilder(TTree& t, const Descriptor* d, RootToMessageFactory* f) :
        _tree(t), _descriptor(d), _factory(f)
    {}

    /// Called when the TTree branch addresses change. 
    /// Generates a new message factory for the _tree.
    Bool_t Notify() 
    { 
        assert(_descriptor);
        (*_factory) = make_message_factory(_tree, _descriptor);
    }
};

/// Copies `tree` into the `stream` using information taken from the compiled in
/// Event class.
void copy_tree(TTree& tree, shared<A4OutputStream> stream, Long64_t entries = -1)
{
    Long64_t tree_entries = tree.GetEntries();
    if (entries > tree_entries)
        entries = tree_entries;
    if (entries < 0)
        entries = tree_entries;
    if (!entries)
        return;
        
    std::cout << "Will process " << entries << " entries" << std::endl;
    
    // Nothing to do!
    if (!entries)
        return;
    
    // Disable all branches. Branches get enabled through 
    // TBranch::ResetBit(kDoNotProcess) we make the message factory.
    tree.SetBranchStatus("*", false);
    
    // An event_factory is automatically created when the branch pointers change
    // through the Tree Notify() call.
    RootToMessageFactory event_factory;
    
    // This is the only place where we say that we're wanting to build the 
    // Event class.
    EventFactoryBuilder builder(tree, Event::descriptor(), &event_factory);
    
    tree.SetNotify(&builder);
    
    size_t total_bytes_read = 0;
    
    for (Long64_t i = 0; i < entries; i++)
    {
        size_t read_data = tree.GetEntry(i);
        total_bytes_read += read_data;
        if (i % 100 == 0)
            std::cout << "Progress " << i << " / " << entries << " (" << read_data << ")" << std::endl;
        
        // Write out one event.
        stream->write(*event_factory());
    }
    
    //Metadata m;
    //m.set_total_events(entries);
    //stream->metadata(m);
    
    std::cout << "Copied " << entries << " entries (" << total_bytes_read << ")" << std::endl;
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


