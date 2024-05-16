#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "CTEST_FULL_OUTPUT\n"; 
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
