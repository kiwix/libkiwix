#include "gtest/gtest.h"
#include "../src/libtorrent.h"

#include <string>

namespace
{

TEST(LibTorrentTest, CanInstantiate)
{
  EXPECT_NO_THROW({ kiwix::LibTorrent lt; });
}

TEST(LibTorrentTest, CanGetVersion)
{
  kiwix::LibTorrent lt;
  std::string version = lt.getVersion();

  // Version should be non-empty
  EXPECT_FALSE(version.empty());

  // Version should contain digits (e.g., "2.0.11")
  EXPECT_NE(version.find_first_of("0123456789"), std::string::npos);
}

} // unnamed namespace
