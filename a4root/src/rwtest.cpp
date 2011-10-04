#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout; using std::endl;

#include <boost/function.hpp>
#include <boost/bind.hpp>
//using namespace boost;
using boost::bind;
using boost::function;

#include <a4/output.h>
#include <a4/input.h>

#include <TLeaf.h>
#include <TChain.h>

#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"
using google::protobuf::Message;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Reflection;

#include "Event.pb.h"

//using namespace std;
using namespace a4::io;

using namespace google;

template<typename T>
class FieldSetter
{
public:
    typedef function<void (const Reflection*, Message*, const FieldDescriptor*, T)> type;
};

typedef function<shared<Message> ()> MessageFactory;
typedef function<void (Message*)>    Copier;
typedef vector<Copier>               Copiers;

A4RegisterClass(Event);
A4RegisterClass(Metadata);

const vector<const FieldDescriptor*> get_fields(const Descriptor* d) {
    vector<const FieldDescriptor*> result;
    for (int i = 0; i < d->field_count(); i++)
        result.push_back(d->field(i));
    return result;
}

void message_from_tree(TTree& tree, protobuf::Message& message, const string& prefix="")
{
    const Descriptor* desc = message.GetDescriptor();
    const Reflection* refl = message.GetReflection();
    
    assert(!desc->nested_type_count()); // unsupported
    
    foreach (auto field, get_fields(desc)) {
        if (field->options().HasExtension(root_branch))
        {
            TLeaf* b = tree.GetLeaf(field->options().GetExtension(root_branch).c_str());
            //cout << "Field : " << field->full_name() << endl;
            //cout << endl;
            
            refl->SetInt32(&message, field, *reinterpret_cast<int*>(b->GetValuePointer()));
        } else if (field->options().HasExtension(root_prefix))
        {
            
        } 
        
    }
}

shared<Event> event_from_tree(TTree& tree)
{
    auto event = shared<Event>(new Event);

    message_from_tree(tree, *event);

    return event;
}

template<typename T>
void call_setter(
    function<void (T, Message*)> setter, Message* message, void* value)
{
    cout << "Setter called! " << *reinterpret_cast<T*>(value) << endl;
    setter(*reinterpret_cast<T*>(value), message);
}

void null_copier(Message*) {}

//template<typename T> Copier make_field_setter(void* p, const FieldDescriptor* f, const Reflection* r) { return null_copier; }
template<typename T>         Copier make_field_setter(T* p, const FieldDescriptor* f, const Reflection* r) 
{
    function<void (T, Message*)> setter = bind(&Reflection::SetInt32, r, _2, f, _1);
    function<void (function<void (T, Message*)>, Message*, T*)> ff = call_setter<T>;
    return bind(ff, setter, _1, p);
}

template<typename T> typename FieldSetter<T>::type reflection_setter() { throw "Don't use this one"; };
template<> FieldSetter<int>::type reflection_setter<int>() { return bind(&Reflection::SetInt32, _1, _2, _3, _4); };
template<> FieldSetter<float>::type reflection_setter<float>() { cout << "Binding Float" << endl; return bind(&Reflection::SetFloat, _1, _2, _3, _4); };
template<> FieldSetter<double>::type reflection_setter<double>() { cout << "Binding Double" << endl; return bind(&Reflection::SetDouble, _1, _2, _3, _4); };

template<typename T>
Copier generic_make_field_setter(void* p, const FieldDescriptor* f, const Reflection*& r)
{
    typename FieldSetter<T>::type test = reflection_setter<T>();
    function<void (T, Message*)> set_reflection_field = bind(test, r, _2, f, _1);
    function<void (function<void (T, Message*)>, Message*, void*)> set_pointer_inside_message = call_setter<T>;
    return bind(set_pointer_inside_message, set_reflection_field, _1, p);
}

shared<Message> message_factory(const Copiers& copiers)
{
    Event* event = new Event;
    
    //c(event);
    for (auto copier: copiers) copier(event);
    
    return shared<Message>(event);
}

template<typename T>
MessageFactory make_message_factory(TTree& tree)
{

    const Reflection* refl = Event::default_instance().GetReflection();
    
    static int x = 1337;
    static double fl = 3.1415;
        
    Copiers copiers;
    copiers.push_back(generic_make_field_setter<int>((void*)&x, Event::descriptor()->FindFieldByName("run_number"), refl));
    //copiers.push_back(generic_make_field_setter<double>((void*)&fl, Event::descriptor()->FindFieldByName("mc_event_weight"), refl));
    
    return bind(message_factory, copiers);
}

void copy_tree(TTree& tree, shared<A4OutputStream> stream, Long64_t entries = -1)
{
    if (entries < 0)
        entries = tree.GetEntries();
    
    MessageFactory event_factory = make_message_factory<Event>(tree);
    
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


