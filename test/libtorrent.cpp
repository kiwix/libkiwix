#include "../src/libtorrent.h"

#include <string>

#include "gtest/gtest.h"

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

TEST(LibTorrentTest, CanIncludeLinkLibtorrent)
{
  // This test verifies that we can successfully:
  // 1. Include libtorrent headers (done in libtorrent.h/cpp)
  // 2. Link against libtorrent library (verified by instantiation)
  // 3. Call basic libtorrent functions (getVersion uses LIBTORRENT_VERSION)

  kiwix::LibTorrent lt;
  std::string version = lt.getVersion();

  // If we can create an instance and get version, linking works
  EXPECT_TRUE(true);
}

}  // unnamed namespace
