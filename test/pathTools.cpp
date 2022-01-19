/*
 * Copyright (C) 2019 Matthieu Gautier
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "gtest/gtest.h"
#include <string>
#include <vector>
#ifndef _WIN32
# include <unistd.h>
# include <fcntl.h>
#endif
#include "../include/tools.h"
#include "../src/tools/pathTools.h"

#ifdef _WIN32
# define S "\\"
# define AS "c:"
# define A_SAMBA "\\\\sambadir"
#else
# define S "/"
# define AS ""
#endif

#define P2(a, b) a S b
#define P3(a, b, c) P2(P2(a, b), c)
#define P4(a, b, c, d) P2(P3(a, b, c), d)
#define P5(a, b, c, d, e) P2(P4(a, b, c, d), e)
#define P6(a, b, c, d, e, f) P2(P5(a, b, c ,d, e), f)

#define A1(a) P2(AS,a)
#define A2(a, b) A1(P2(a, b))
#define A3(a, b, c) A1(P3(a, b, c))
#define A4(a, b, c, d) A1(P4(a, b, c, d))
#define A5(a, b, c, d, e) A1(P5(a, b, c, d, e))

std::vector<std::string> normalizeParts(std::vector<std::string>& parts, bool absolute);
std::vector<std::string> nParts(std::vector<std::string> parts, bool absolute) {
  return normalizeParts(parts, absolute);
}
#ifdef _WIN32
std::wstring Utf8ToWide(const std::string& str);
std::string WideToUtf8(const std::wstring& wstr);
#endif

namespace
{

#define V std::vector<std::string>
TEST(pathTools, normalizePartsAbsolute)
{
#define N(...) nParts(__VA_ARGS__, true)
  ASSERT_EQ(N({}), V({}));
#ifdef _WIN32
  ASSERT_EQ(N({"c:"}), V({"c:"}));
#else
  ASSERT_EQ(N({""}), V({"", ""}));
#endif
  ASSERT_EQ(N({AS, "a"}), V({AS, "a"}));
  ASSERT_EQ(N({AS, "a", "b"}), V({AS, "a", "b"}));
  ASSERT_EQ(N({AS, "a", "b", ".."}), V({AS, "a"}));
#ifdef _WIN32
  ASSERT_EQ(N({AS, "a", "b", "..", ".."}), V({AS}));
#else
  ASSERT_EQ(N({AS, "a", "b", "..", ".."}), V({AS, ""}));
#endif
  ASSERT_EQ(N({AS, "a", "b", "..", "..", "..", "foo"}), V({AS, "foo"}));
  ASSERT_EQ(N({AS, "..", "..", "c", "d", ".", "..", "foo"}), V({AS, "c", "foo"}));
  ASSERT_EQ(N({AS, "a", "b", ".", "c", "d", "..", "foo"}), V({AS, "a", "b", "c", "foo"}));

#ifdef _WIN32
  ASSERT_EQ(N({"c:", "a", "b", ".", "c", "d:", "..", "foo"}), V({"d:", "foo"}));
  ASSERT_EQ(N({"","","samba","a","b"}), V({"\\\\samba", "a", "b"}));
#endif
#undef N
}

TEST(pathTools, normalizePartsRelative)
{
#define N(...) nParts(__VA_ARGS__, false)
  ASSERT_EQ(N({}), V({}));
  ASSERT_EQ(N({""}), V({}));
  ASSERT_EQ(N({"a"}), V({"a"}));
  ASSERT_EQ(N({"a", "b"}), V({"a", "b"}));
  ASSERT_EQ(N({"a", "b", ".."}), V({"a"}));
  ASSERT_EQ(N({"a", "b", "..", ".."}), V({}));
  ASSERT_EQ(N({"a", "b", "..", "..", "..", "foo"}), V({"..", "foo"}));
  ASSERT_EQ(N({"..", "..", "c", "d", ".", "..", "foo"}), V({"..", "..", "c", "foo"}));
  ASSERT_EQ(N({"a", "b", ".", "c", "d", "..", "foo"}), V({"a", "b", "c", "foo"}));
#undef N
}

TEST(pathTools, isRelativePath)
{
  ASSERT_TRUE(kiwix::isRelativePath("foo"));
  ASSERT_TRUE(kiwix::isRelativePath(P2("foo","bar")));
  ASSERT_TRUE(kiwix::isRelativePath(P3(".","foo","bar")));
  ASSERT_TRUE(kiwix::isRelativePath(P2("..","foo")));
  ASSERT_TRUE(kiwix::isRelativePath(P4("foo","","bar","")));
  ASSERT_FALSE(kiwix::isRelativePath(A1("foo")));
  ASSERT_FALSE(kiwix::isRelativePath(A2("foo", "bar")));
#ifdef _WIN32
  ASSERT_FALSE(kiwix::isRelativePath(P2(A_SAMBA, "foo")));
  ASSERT_FALSE(kiwix::isRelativePath(P3(A_SAMBA, "foo", "bar")));
#endif
}

TEST(pathTools, computeAbsolutePath)
{
  ASSERT_EQ(kiwix::computeAbsolutePath(A2("a","b"), "foo"),
            A3("a","b","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b",""), "foo"),
            A3("a","b","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A2("a","b"), P2(".","foo")),
            A3("a","b","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A2("a","b"), P2("..","foo")),
            A2("a","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b",""), P2("..","foo")),
            A2("a","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A5("a","b","c","d","e"), P2("..","foo")),
            A5("a","b","c","d","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A5("a","b","c","d","e"), P5("..","..","..","g","foo")),
            A4("a","b","g","foo"));
#ifdef _WIN32
  ASSERT_EQ(kiwix::computeAbsolutePath(P4(A_SAMBA,"a","b",""), P2("..","foo")),
            P3(A_SAMBA,"a","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(P6(A_SAMBA,"a","b","c","d","e"), P5("..","..","..","g","foo")),
            P5(A_SAMBA,"a","b","g","foo"));
#endif
}

TEST(pathTools, computeRelativePath)
{
  ASSERT_EQ(kiwix::computeRelativePath(A2("a","b"), A3("a","b","foo")),
            "foo");
  ASSERT_EQ(kiwix::computeRelativePath(A3("a","b",""), A3("a","b","foo")),
            "foo");
  ASSERT_EQ(kiwix::computeRelativePath(A2("a","b"), A2("a","foo")),
            P2("..","foo"));
  ASSERT_EQ(kiwix::computeRelativePath(A3("a","b",""), A2("a","foo")),
            P2("..","foo"));
  ASSERT_EQ(kiwix::computeRelativePath(A5("a","b","c","d","e"), A5("a","b","c","d","foo")),
            P2("..","foo"));
  ASSERT_EQ(kiwix::computeRelativePath(A5("a","b","c","d","e"), A4("a","b","g","foo")),
            P5("..","..","..","g","foo"));
#ifdef _WIN32
  ASSERT_EQ(kiwix::computeRelativePath(P3(A_SAMBA,"a","b"), P3(A_SAMBA,"a","foo")),
            P2("..","foo"));
  ASSERT_EQ(kiwix::computeRelativePath(P6(A_SAMBA,"a","b","c","d","e"), P5(A_SAMBA,"a","b","g","foo")),
            P5("..","..","..","g","foo"));

#endif
}

TEST(pathTools, removeLastPathElement)
{
  ASSERT_EQ(kiwix::removeLastPathElement(P3("a","b","c")),
            P2("a","b"));
  ASSERT_EQ(kiwix::removeLastPathElement(A3("a","b","c")),
            A2("a","b"));
  ASSERT_EQ(kiwix::removeLastPathElement(P4("a","b","c","")),
            P2("a","b"));
  ASSERT_EQ(kiwix::removeLastPathElement(A4("a","b","c","")),
            A2("a","b"));
}

TEST(pathTools, appendToDirectory)
{
  ASSERT_EQ(kiwix::appendToDirectory(P3("a","b","c"), "foo.xml"),
            P4("a","b","c","foo.xml"));
  ASSERT_EQ(kiwix::appendToDirectory(P4("a","b","c",""), "foo.xml"),
            P4("a","b","c","foo.xml"));
  ASSERT_EQ(kiwix::appendToDirectory(P3("a","b","c"), P2("d","foo.xml")),
            P5("a","b","c","d","foo.xml"));
  ASSERT_EQ(kiwix::appendToDirectory(P4("a","b","c",""), P2("d","foo.xml")),
            P5("a","b","c","d","foo.xml"));
  ASSERT_EQ(kiwix::appendToDirectory(P3("a","b","c"), P2(".","foo.xml")),
            P5("a","b","c",".","foo.xml"));
  ASSERT_EQ(kiwix::appendToDirectory(P4("a","b","c",""), P2(".","foo.xml")),
            P5("a","b","c",".","foo.xml"));
}

TEST(pathTools, fileExists)
{
  ASSERT_TRUE(kiwix::fileExists(P3(".","test","example.zim")));
  ASSERT_FALSE(kiwix::fileExists(P3(".","test","noFile.zim")));
}

TEST(pathTools, fileReadable)
{
  ASSERT_TRUE(kiwix::fileReadable(P3(".","test","example.zim")));
  ASSERT_FALSE(kiwix::fileReadable(P3(".","test","noFile.zim")));
#ifdef _POSIX_SOURCE
  std::string path = P3(".","test","example.zim");
  auto myprivs = geteuid();
  if (myprivs != 0) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd != -1) {
      int wResp = fchmod(fd, ~(S_IRWXU | S_IRWXG | S_IRWXO)); // remove all permissions
      if (wResp == 0) {
        EXPECT_FALSE(kiwix::fileReadable(P3(".","test","example.zim")));
      }
      int resetResp = fchmod(fd, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP); // reset back permissions to -rw-rw-r--
      if (resetResp == 0) {
        EXPECT_TRUE(kiwix::fileReadable(P3(".","test","example.zim")));
      }
    }
  }
#endif
}

TEST(pathTools, goUp)
{
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), ".."),
            A2("a", "b"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P2("..","..")),
            A1("a"));
#ifdef _WIN32
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P3("..","..","..")),
            "c:");
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P4("..","..","..","..")),
            "c:");
  ASSERT_EQ(kiwix::computeAbsolutePath(P4(A_SAMBA,"a","b","c"), ".."),
            P3(A_SAMBA,"a", "b"));
  ASSERT_EQ(kiwix::computeAbsolutePath(P4(A_SAMBA,"a","b","c"), P2("..","..")),
            P2(A_SAMBA,"a"));
  ASSERT_EQ(kiwix::computeAbsolutePath(P4(A_SAMBA,"a","b","c"), P3("..","..","..")),
            A_SAMBA);
  ASSERT_EQ(kiwix::computeAbsolutePath(P4(A_SAMBA,"a","b","c"), P4("..","..","..","..")),
            A_SAMBA);

#else
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P3("..","..","..")),
            "/");
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P4("..","..","..","..")),
            "/");
#endif


  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P2("..", "foo")),
            A3("a", "b","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P3("..","..","foo")),
            A2("a","foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P4("..","..","..","foo")),
            A1("foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(A3("a","b","c"), P5("..","..","..","..","foo")),
            A1("foo"));
#ifdef _WIN32
  ASSERT_EQ(kiwix::computeAbsolutePath(P4(A_SAMBA,"a","b","c"), P4("..","..","..","foo")),
            P2(A_SAMBA,"foo"));
  ASSERT_EQ(kiwix::computeAbsolutePath(P4(A_SAMBA,"a","b","c"), P5("..","..","..","..","foo")),
            P2(A_SAMBA,"foo"));
#endif
}


#ifdef _WIN32
TEST(pathTools, dirChange)
{
  std::string p1("c:\\a\\b\\c");
  std::string p2("d:\\d\\e\\foo.xml");
  {
    std::string relative_path = kiwix::computeRelativePath(p1, p2);
    ASSERT_EQ(relative_path, p2);
    std::string abs_path = kiwix::computeAbsolutePath(p1, relative_path);
    ASSERT_EQ(abs_path, p2);
    ASSERT_EQ(kiwix::computeAbsolutePath(p1, "..\\..\\..\\..\\..\\d:\\d\\e\\foo.xml"), p2);
  }
  std::string ps("\\\\samba\\d\\e\\foo.xml");
  {
    std::string relative_path = kiwix::computeRelativePath(p1, ps);
    ASSERT_EQ(relative_path, ps);
    std::string abs_path = kiwix::computeAbsolutePath(p1, relative_path);
    ASSERT_EQ(abs_path, ps);
    // I'm not sure this test is valid on windows :/
//    ASSERT_EQ(kiwix::computeAbsolutePath(p1, "..\\..\\..\\..\\..\\\\samba\\d\\e\\foo.xml"), ps);
  }
}

TEST(pathTools, Utf8ToWide)
{
  ASSERT_EQ(Utf8ToWide(u8""), L"");
  ASSERT_EQ(Utf8ToWide(u8"test"), L"test");
  ASSERT_EQ(Utf8ToWide(u8"testé`œà"), L"testé`œà");
}

TEST(pathTools, WideToUtf8)
{
  ASSERT_EQ(WideToUtf8(L""), u8"");
  ASSERT_EQ(WideToUtf8(L"test"), u8"test");
  ASSERT_EQ(WideToUtf8(L"testé`œà"), u8"testé`œà");
}
#endif


};
