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
#include <a4/exceptions.h>
using a4::Fatal;

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



template<typename SOURCE_TYPE, typename PROTOBUF_TYPE>
void call_setter(typename Setter<PROTOBUF_TYPE>::msg setter, Message* message, void* value)
{
    setter(message, static_cast<PROTOBUF_TYPE>(*reinterpret_cast<SOURCE_TYPE*>(value)));
}

template<typename T> typename Setter<T>::type reflection_setter() { 
    throw Fatal("Unknown type: ", typeid(T), ", add a DEFINE_SETTER line in ", __FILE__); 
};
#define DEFINE_SETTER(T, SetterName) \
    template<> Setter<T>::type reflection_setter<T>() { \
        return bind(&::google::protobuf::Reflection::SetterName, _1, _2, _3, _4); \
    }
    
DEFINE_SETTER(int32_t,     SetInt32);
DEFINE_SETTER(int64_t,     SetInt64);
DEFINE_SETTER(uint32_t,    SetUInt32);
DEFINE_SETTER(uint64_t,    SetUInt64);
DEFINE_SETTER(float,       SetFloat);
DEFINE_SETTER(double,      SetDouble);
DEFINE_SETTER(bool,        SetBool);
DEFINE_SETTER(std::string, SetString);

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
    function<void (typename Setter<T>::msg, Message*, void*)> set_pointer_inside_message = call_setter<T, T>;
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
            throw Fatal("Unknown field type in copier_from_field_descriptor ", f->cpp_type());
    }
            
    #undef FIELD
}

shared<Message> message_factory(const Message* default_instance, const Copiers& copiers)
{
    shared<Message> message(default_instance->New());
    
    foreach (const Copier& copier, copiers) copier(message.get());
    
    return message;
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
    const size_t count = compute_count();
    
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

template<typename SOURCE_TYPE, typename PROTOBUF_TYPE>
void submessage_setter(Message** messages, size_t count, TBranchElement* br, 
    const typename Setter<PROTOBUF_TYPE>::msg& setter, const FieldDescriptor* field)
{
    vector<SOURCE_TYPE>* values = reinterpret_cast<vector<SOURCE_TYPE>* >(br->GetObject());
    for (size_t i = 0; i < count; i++)
        setter(messages[i], static_cast<PROTOBUF_TYPE>(values->at(i)));
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
function<size_t ()> make_count_getter(TBranchElement* branch_element)
{
    #define TRY_MATCH(ctype) \
        if (branch_typename == "vector<" #ctype ">") \
            return bind(count_getter<ctype>, branch_element)
    
    const std::string branch_typename = branch_element->GetTypeName();
    
    TRY_MATCH(int);
    TRY_MATCH(long);
    TRY_MATCH(short);
    TRY_MATCH(char);
    TRY_MATCH(unsigned int);
    TRY_MATCH(unsigned long);
    TRY_MATCH(unsigned short);
    TRY_MATCH(unsigned char);
    TRY_MATCH(unsigned long long);
    TRY_MATCH(float);
    TRY_MATCH(double);
    TRY_MATCH(std::string);
    
    throw Fatal("a4root doesn't know how to count the ", 
        "number of elements in a \"", branch_typename, "\" from a ROOT TTree. "
        "If this should be possible please contact the A4 developers, " 
        "or fix it yourself at " __FILE__ ":", __LINE__, ".");
    
    #undef TRY_MATCH    
}

/// Returns a function which will set one `field` of a protobuf 
// "repeated Message" from the vector branch `branch_element`.
SubmessageSetter make_submessage_setter(TBranchElement* branch_element, 
    const FieldDescriptor* field, const Reflection* refl)
{
    // Return a function which will copy elements from `branch_element` into the
    // `field` of message `_1`, assuming it is a vector<T> of length `_2`.
    // This uses the `refl` argument.
    #define BIND(source_type, destination_type) \
        bind(submessage_setter<source_type, destination_type>, _1, _2, \
             branch_element, make_setter<destination_type>(field, refl), field)
    
    // !Note!: `check_typestring` is expanded as a string!!
    #define TRY_MATCH(source_type, protobuf_destination_type) \
        if (branch_typename == "vector<" #source_type ">" ) \
        { \
            std::cerr << "Matching " << branch_element->GetName() << " (" \
                      << branch_typename << ") to " << field->full_name() \
                      << " (" << str_cat(typeid(protobuf_destination_type)) << ")" << std::endl; \
            return BIND(source_type, protobuf_destination_type); \
        }
    
    // This happens when we don't know how to deal with the combination of 
    // {field->cpp_type(), root branch type}
    #define FAILURE(protobuf_typename) \
        throw Fatal(str_cat("a4root doesn't know how to convert the branch ", \
            branch_element->GetName(), " to the protobuf field ", \
            field->full_name(), " with types ROOT=", branch_typename, " and " \
            "protobuf=", protobuf_typename, ". The .proto is probably wrong. " \
            "Alternatively, if the types are compatible and conversion should " \
            "be possible please contact the A4 developers, " \
            "or fix it yourself at " __FILE__ ":", __LINE__, "."))
    
    const std::string branch_typename = branch_element->GetTypeName();
    
    switch (field->cpp_type())
    {
        case FieldDescriptor::CPPTYPE_BOOL:
            TRY_MATCH(int,            bool);
            TRY_MATCH(long,           bool);
            TRY_MATCH(short,          bool);
            TRY_MATCH(char,           bool);
            TRY_MATCH(unsigned int,   bool);
            TRY_MATCH(unsigned long,  bool);
            TRY_MATCH(unsigned short, bool);
            TRY_MATCH(unsigned char,  bool);
            FAILURE("FieldDescriptor::CPPTYPE_BOOL");
        
        case FieldDescriptor::CPPTYPE_INT32:
            TRY_MATCH(int,   int32_t);
            TRY_MATCH(long,  int32_t);
            TRY_MATCH(short, int32_t);
            TRY_MATCH(char,  int32_t);
            FAILURE("FieldDescriptor::CPPTYPE_INT32");
        
        case FieldDescriptor::CPPTYPE_UINT32:
            TRY_MATCH(unsigned int,   uint32_t);
            TRY_MATCH(unsigned long,  uint32_t);
            TRY_MATCH(unsigned short, uint32_t);
            TRY_MATCH(unsigned char,  uint32_t);
            FAILURE("FieldDescriptor::CPPTYPE_UINT32");
        
        case FieldDescriptor::CPPTYPE_INT64:
            TRY_MATCH(int,   int64_t);
            TRY_MATCH(long,  int64_t);
            TRY_MATCH(short, int64_t);
            TRY_MATCH(char,  int64_t);
            FAILURE("FieldDescriptor::CPPTYPE_INT64");
            
        case FieldDescriptor::CPPTYPE_UINT64:
            TRY_MATCH(unsigned long long, uint64_t);
            TRY_MATCH(unsigned int,       uint64_t);
            TRY_MATCH(unsigned long,      uint64_t);
            TRY_MATCH(unsigned short,     uint64_t);
            TRY_MATCH(unsigned char,      uint64_t);
            FAILURE("FieldDescriptor::CPPTYPE_UINT64");
        
        case FieldDescriptor::CPPTYPE_FLOAT:
            TRY_MATCH(float, float);
            FAILURE("FieldDescriptor::CPPTYPE_FLOAT");
            
        case FieldDescriptor::CPPTYPE_DOUBLE:
            TRY_MATCH(double, double);
            TRY_MATCH(float,  double);
            FAILURE("FieldDescriptor::CPPTYPE_DOUBLE");
        
        case FieldDescriptor::CPPTYPE_STRING:
        {
            using std::string;
            TRY_MATCH(string, std::string);
            FAILURE("FieldDescriptor::CPPTYPE_STRING");
        }
        
        default:
            FAILURE(str_cat("field with typecode", field->cpp_type()));
    }
    
    #undef TRY_MATCH
    #undef FAILURE
    #undef BIND
}

/// Used to indicate that that it is not possible to copy this message.
/// Gives an assertion failure if called.
void null_copier(Message*) { throw Fatal("null_coper called. This should never happen."); }

Copier make_submessage_factory(TTree& tree, const Reflection* parent_refl, 
    const FieldDescriptor* parent_field, const std::string& prefix="")
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
            //throw Fatal(warning);
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
            throw Fatal("Can't currently convert ", 
                field->full_name(), " with ROOT branch ", leafname, " type ",
                br->GetTypeName());
            continue; // TODO(pwaller): it's a vector<vector<...
        }            

        // Re-enable this branch
        br->ResetBit(kDoNotProcess);
        
        // Do this exactly once over the loop over get_fields(desc).
        if (!compute_count)
            compute_count = make_count_getter(br);
        
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
                    //throw Fatal(warning);
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
                throw Fatal(field->full_name(), 
                    " is not repeated but has a [(root_prefix=\"", prefix, "\")]. "
                    "These are not compatible with one-another.");
            } else {
                // What to do here? Warn the user that we're ignoring the field?
                const std::string warning = str_cat(field->full_name(), 
                    " has no conversion specifier, e.g. [(root_branch=\"", 
                    field->name(), "\")];.");
                std::cerr << warning << std::endl;
                continue;
                //throw Fatal(warning);
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


