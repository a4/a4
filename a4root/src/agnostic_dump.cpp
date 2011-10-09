#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl;

#include <google/protobuf/text_format.h>
#include <google/protobuf/descriptor.h>
//namespace google::protobuf { using DescriptorPool; }
using google::protobuf::DescriptorPool;
using google::protobuf::FileDescriptor;

#include <a4/output.h>
#include <a4/input.h>

#include "Event.pb.h"

using namespace a4::io;


// *ONLY* needed at the moment to stop the stream from complaining and falling over
// (this is a big 'only').
//A4RegisterClass(a4::example::root::Event);
//A4RegisterClass(a4::example::root::Metadata);

int main(int argc, char ** argv) {
    A4Input in;
    in.add_file("test_io.a4");
    while (shared<A4InputStream> stream = in.get_stream()) {
        // Needed because the header isn't read before we try and read an event
        
        A4Message m = stream->next();
        
        //cout << "I have a message with classid " << m.class_id << endl;
        string str;
        google::protobuf::TextFormat::PrintToString(*m.message, &str);
        cout << str << endl;
        
        //DescriptorPool pool(DescriptorPool::generated_pool());
        //const FileDescriptor* file = pool.BuildFile(stream->_content_descriptor_proto);
        //if (file)
            //std::cout << file->DebugString() << std::endl;
        while (A4Message msg = stream->next()); // Stream out
    }
}

