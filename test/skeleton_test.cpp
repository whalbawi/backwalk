#include "skeleton.h"

#include "gtest/gtest.h"

class SkeletonTest : public ::testing::Test {
  protected:
    skeleton::Skeleton skeleton{};
};

TEST_F(SkeletonTest, Adder) {
    skeleton.add(1, 2);
    ASSERT_EQ(skeleton.get_result(), 3);
}

TEST_F(SkeletonTest, Version) {
    ASSERT_NE(skeleton::Skeleton::get_version(), "");
}
