#include <gtest/gtest.h>
#include <a4/histogram.h>

using namespace a4::hist;

TEST(a4hist, h1) {
    H1 h1;
    h1.constructor(100,0,1);
    h1.fill(0.1,1.0);
    h1.fill(0.9,1.0);
    ASSERT_EQ(2.0, h1.integral());

}
