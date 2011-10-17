#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl;

#include <google/protobuf/text_format.h>
#include <google/protobuf/descriptor.h>
using google::protobuf::DescriptorPool;
using google::protobuf::FileDescriptor;

//#include <a4/output.h>
#include <a4/input.h>

//using namespace a4::io;


int main(int argc, char ** argv) {
    a4::io::A4Input in;
    in.add_file("test_io.a4");
    
    shared<a4::io::InputStream> stream = in.get_stream();
    // Needed because the header isn't read before we try and read an event
    
    a4::io::A4Message m = stream->next();
    
    string str;
    google::protobuf::TextFormat::PrintToString(*m.message, &str);
    cout << str << endl;
}

