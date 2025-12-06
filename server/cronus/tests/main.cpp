#include <gtest/gtest.h>

// Provide a test runner main since we did not link against GTest::gtest_main
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}