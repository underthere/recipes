#include <gtest/gtest.h>

#include "src/RingBuffer.h"

using namespace underthere::ds;

TEST(RingBuffer, test) {
    RingBuffer<int> rb(3);

    EXPECT_TRUE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 0);

    EXPECT_TRUE(rb.push(1));
    EXPECT_FALSE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 1);

    EXPECT_TRUE(rb.push(2));
    EXPECT_FALSE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 2);

    EXPECT_TRUE(rb.push(3));
    EXPECT_FALSE(rb.empty());
    EXPECT_TRUE(rb.full());
    EXPECT_EQ(rb.size(), 3);

    EXPECT_FALSE(rb.push(4));
    EXPECT_FALSE(rb.empty());
    EXPECT_TRUE(rb.full());
    EXPECT_EQ(rb.size(), 3);

    EXPECT_EQ(rb.pop(), 1);
    EXPECT_FALSE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 2);

    EXPECT_EQ(rb.pop(), 2);
    EXPECT_FALSE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 1);

    EXPECT_EQ(rb.pop(), 3);
    EXPECT_TRUE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 0);

    EXPECT_FALSE(rb.pop().has_value());
}
