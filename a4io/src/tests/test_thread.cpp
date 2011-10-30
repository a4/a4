#include <iostream>
#include <functional>
#include <boost/thread.hpp>

#include "a4/io/A4Stream.pb.h"
#include "a4/output.h"
#include "a4/input.h"

using namespace std;
using namespace a4::io;

const int N = 1000;

void no_write(A4Output &a4o) {
    shared<OutputStream> stream = a4o.get_stream();
}

void my_write(A4Output &a4o) {
    shared<OutputStream> stream = a4o.get_stream();

    TestEvent e;
    for(int i = 0; i < N; i++) {
        e.set_event_number(i);
        stream->write(e);
    }
    TestMetaData m;
    m.set_meta_data(N);
    stream->metadata(m);
}

void my_read(A4Input &in) {
    shared<InputStream> stream = in.get_stream();
    if (!stream) return;
    int cnt = 0;
    while (A4Message msg = stream->next()) {
        if (shared<TestEvent> te = msg.as<TestEvent>()) {
            assert((cnt++%N) == te->event_number());
        }
    }
    if (stream->error()) std::cerr << "stream error in thread " << boost::this_thread::get_id() << std::endl;
    assert(cnt == 5*N);
}

void no_read(A4Input &in) {
    shared<InputStream> stream = in.get_stream();
    if (!stream) return;
    assert(stream->good());
    if (stream->error()) {
        std::cerr << "stream error in no_read thread " << boost::this_thread::get_id() << std::endl;
    }
}

int main(int argc, char ** argv) {
    {
        A4Output a4o("test_thread.a4", "TestEvent");
        boost::thread t1(my_write, boost::ref(a4o));
        boost::thread t2(my_write, boost::ref(a4o));
        boost::thread t3(my_write, boost::ref(a4o));
        boost::thread t4(my_write, boost::ref(a4o));
        boost::thread t5(my_write, boost::ref(a4o));
        boost::thread t6(no_write, boost::ref(a4o));
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();
    }
    {
        A4Input in;
        in.add_file("test_thread.a4");
        in.add_file("test_thread.a4");
        in.add_file("test_thread.a4");
        in.add_file("test_thread.a4");
        in.add_file("test_thread.a4");
        boost::thread t1(no_read, boost::ref(in));
        boost::thread t2(my_read, boost::ref(in));
        boost::thread t3(my_read, boost::ref(in));
        boost::thread t4(my_read, boost::ref(in));
        boost::thread t5(my_read, boost::ref(in));
        boost::thread t6(my_read, boost::ref(in));
        boost::thread t7(my_read, boost::ref(in));
        boost::thread t8(my_read, boost::ref(in));
        boost::thread t9(my_read, boost::ref(in));
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();
        t7.join();
        t8.join();
        t9.join();
    }
}
