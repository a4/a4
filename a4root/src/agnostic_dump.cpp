#include <iostream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>

#include <a4/output.h>
#include <a4/input.h>

using namespace std;
using namespace a4::io;
using namespace google::protobuf;

int main(int argc, char ** argv) {
    A4Input in;
    in.add_file("test_io.a4");
    while (shared<A4InputStream> stream = in.get_stream()) {
        A4Message msg = stream->next();
        DescriptorPool pool(DescriptorPool::generated_pool());
        const FileDescriptor* file = pool.BuildFile(stream->_content_descriptor_proto);
        cout << file->DebugString() << endl;
    }
}

