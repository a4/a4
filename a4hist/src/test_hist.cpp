#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include <a4/histogram.h>

using namespace a4::hist;

const size_t GRIND_REPETITIONS = 20000000;

TEST(a4hist, h1) {
    H1 h1;
    h1(100,0,1);
    h1.fill(0.1, 1.0);
    h1.fill(0.9, 1.0);
    ASSERT_EQ(2.0, h1.integral());
}


TEST(a4hist, h2) {
    H2 h2;
    h2("Title")(100, 0, 1,"x")(100, 0, 1,"y");
    h2.fill(0.1, 0.1);
    h2.fill(0.2, 0.1);
    ASSERT_EQ(2, h2.integral());
}

TEST(a4hist, basic_variable_binning_check) {
    H1 h1;
    
    h1("Title")({1., 2., 4., 8., 16.});
    
    unsigned int n = 0;
    for (double i: {1., 1.1, 4., 8., 9., 10., 11., 16.}) { h1.fill(i); n++; }
    
    ASSERT_EQ(n, h1.integral());
    
    // -------
    
    H2 h2;
    h2("Title")({1., 2., 4., 8., 16.})({1., 2., 4., 8., 16.});
    
    n = 0;
    for (double i: {1., 1.1, 4., 8., 9., 10., 11., 16.}) { h2.fill(i, i); n++; }
    
    ASSERT_EQ(n, h2.integral());
    
    // -------
    
    H3 h3;
    h3("Title")({1., 2., 4., 8., 16.})({1., 2., 4., 8., 16.})({1., 2., 4., 8., 16.});
    
    n = 0;
    for (double i: {1., 1.1, 4., 8., 9., 10., 11., 16.}) { h3.fill(i, i, i); n++; }
    
    ASSERT_EQ(n, h3.integral());
}

TEST(a4hist, test_h1_grind) {
    H1 h1;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h1(100, 0, 1).fill(i);
}

TEST(a4hist, test_h1_grind_labelled) {
    H1 h1;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h1("title")(100, 0, 1, "axis label").fill(i);
}

TEST(a4hist, test_h1_grind_variable) {
    H1 h1;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h1({1., 2., 3., 4., 5., 100000., 1000000., 5000000.}).fill(i);
}

TEST(a4hist, test_h2_grind) {
    H2 h2;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h2(100, 0, 1)(100, 0, 1).fill(i, i);
}

TEST(a4hist, test_h2_grind_variable) {
    H2 h2;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h2  ({1., 2., 3., 4., 5., 100000., 1000000., 5000000.})
            ({1., 2., 3., 4., 6., 100000., 1000000., 5000000.})
            .fill(i, i);
            
    //std::cout << h2 << std::endl;
}

TEST(a4hist, test_h3_grind) {
    H3 h3;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h3(100, 0, 1)(100, 0, 1)(100, 0, 1).fill(i, i, i);
}

TEST(a4hist, test_h3_grind_variable) {
    H3 h3;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h3  ({1., 2., 3., 4., 5., 100000., 1000000., 5000000.})
            ({1., 2., 3., 4., 6., 100000., 1000000., 5000000.})
            ({1., 2., 3., 4., 7., 100000., 1000000., 5000000.})
            .fill(i, i, i);
            
    //std::cout << h2 << std::endl;
}

TEST(a4hist, test_h3_grind_variable_titles) {
    H3 h3;
    for (size_t i = 0; i < GRIND_REPETITIONS; i++)
        h3  ("title")
            ({1., 2., 3., 4., 5., 100000., 1000000., 5000000.}, "axis 1")
            ({1., 2., 3., 4., 6., 100000., 1000000., 5000000.}, "axis 2")
            ({1., 2., 3., 4., 7., 100000., 1000000., 5000000.}, "axis 3")
            .fill(i, i, i);
            
    //std::cout << h2 << std::endl;
}
