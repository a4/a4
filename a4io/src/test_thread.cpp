#include <iostream>
#include <functional>
#include <thread>

#include "a4/proto/io/A4Stream.pb.h"
#include "a4/output.h"
#include "a4/input.h"

using namespace std;
using namespace a4::io;

const int N = 1000;

void no_write(A4Output &a4o) {
    auto stream = a4o.get_stream();
    stream->content_cls<TestEvent>();
    stream->metadata_cls<TestMetaData>();
}

void my_write(A4Output &a4o) {
    auto stream = a4o.get_stream();
    stream->content_cls<TestEvent>();
    stream->metadata_cls<TestMetaData>();

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
    shared<A4InputStream> stream = in.get_stream();
    if (!stream) return;
    int cnt = 0;
    while (auto msg = stream->next()) {
        if (auto te = msg.as<TestEvent>()) {
            assert((cnt++%N) == te->event_number());
        }
    }
    if (stream->error()) std::cerr << "stream error in thread " << std::this_thread::get_id() << std::endl;
    assert(cnt == 5*N);
}

void no_read(A4Input &in) {
    shared<A4InputStream> stream = in.get_stream();
    if (!stream) return;
    assert(stream->good());
    if (stream->error()) {
        std::cerr << "stream error in no_read thread " << std::this_thread::get_id() << std::endl;
    }
}

int main(int argc, char ** argv) {
    {
        A4Output a4o("test_thread.a4", "TestEvent");
        std::thread t1 = std::thread(std::bind(&my_write, std::ref(a4o)));
        std::thread t2 = std::thread(std::bind(&my_write, std::ref(a4o)));
        std::thread t3 = std::thread(std::bind(&my_write, std::ref(a4o)));
        std::thread t4 = std::thread(std::bind(&my_write, std::ref(a4o)));
        std::thread t5 = std::thread(std::bind(&my_write, std::ref(a4o)));
        std::thread t6 = std::thread(std::bind(&no_write, std::ref(a4o)));
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
        std::thread t1 = std::thread(std::bind(&no_read, std::ref(in)));
        std::thread t2 = std::thread(std::bind(&my_read, std::ref(in)));
        std::thread t3 = std::thread(std::bind(&my_read, std::ref(in)));
        std::thread t4 = std::thread(std::bind(&my_read, std::ref(in)));
        std::thread t5 = std::thread(std::bind(&my_read, std::ref(in)));
        std::thread t6 = std::thread(std::bind(&my_read, std::ref(in)));
        std::thread t7 = std::thread(std::bind(&my_read, std::ref(in)));
        std::thread t8 = std::thread(std::bind(&my_read, std::ref(in)));
        std::thread t9 = std::thread(std::bind(&my_read, std::ref(in)));
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
