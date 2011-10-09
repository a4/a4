#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout; using std::endl;

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
    
    for (auto field: get_fields(desc)) {
        if (!field->options().HasExtension(root_branch))
            continue;
        
        TLeaf* b = tree.GetLeaf(field->options().GetExtension(root_branch).c_str());
        //cout << "Field : " << field->full_name() << endl;
        //cout << endl;
        
        refl->SetInt32(&message, field, *reinterpret_cast<int*>(b->GetValuePointer()));
    }
}

shared<Event> event_from_tree(TTree& tree)
{
    auto event = shared<Event>(new Event);

    message_from_tree(tree, *event);

    return event;
}

void copy_tree(TTree& tree, shared<A4OutputStream> stream, Long64_t entries = -1)
{
    if (entries < 0)
        entries = tree.GetEntries();
        
    for (Long64_t i = 0; i < entries; i++)
    {    
        tree.GetEntry(i);
        
        stream->write(*event_from_tree(tree));
    }
    
    Metadata m;
    m.set_total_events(1);
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


