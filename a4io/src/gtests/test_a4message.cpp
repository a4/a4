#include "a4/message.h"
#include "a4/dynamic_message.h"

#include <a4/io/A4Stream.pb.h>

#include <gtest/gtest.h>

TEST(a4io, a4message) {
    a4::io::TestEvent e;
    e.set_event_number(1234);
    e.set_event_data(5678);
    
    a4::io::A4Message m(e);
    ASSERT_EQ(m.as<a4::io::TestEvent>()->event_number(), e.event_number());
    ASSERT_EQ(m.as<a4::io::TestEvent>()->event_data(), e.event_data());
    
    // Check that the dynamic machinary is working correctly
    ASSERT_EQ(static_cast<decltype(e.event_number())>(m.dynamic_field("event_number")->value()), e.event_number());
    ASSERT_EQ(static_cast<decltype(e.event_data()  )>(m.dynamic_field("event_data")  ->value()), e.event_data());
}

