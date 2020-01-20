#include "gtest/gtest.h"
#include "../include/kiwixserve.h"

TEST(KiwixServeTest, PortTest)
{
    kiwix::KiwixServe kiwixServe("libraryPath", 8181);
    EXPECT_EQ(kiwixServe.getPort(), 8181);
    kiwixServe.setPort(8484);
    EXPECT_EQ(kiwixServe.getPort(), 8484);
    EXPECT_EQ(kiwixServe.setPort(0), -1);
    EXPECT_EQ(kiwixServe.setPort(3456789), -1);
}
