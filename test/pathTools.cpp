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
#include "../include/tools/pathTools.h"

#ifdef _WIN32
# define S "\\"
# define AS "c:"
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

std::vector<std::string> normalizeParts(std::vector<std::string> parts, bool absolute);
#ifdef _WIN32
std::wstring Utf8ToWide(const std::string& str);
std::string WideToUtf8(const std::wstring& wstr);
#endif

namespace
{

#define V std::vector<std::string>
TEST(pathTools, normalizePartsAbsolute)
{
#define N(...) normalizeParts(__VA_ARGS__, true)
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
#endif
#undef N
}

TEST(pathTools, normalizePartsRelative)
{
#define N(...) normalizeParts(__VA_ARGS__, false)
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
  ASSERT_TRUE(isRelativePath("foo"));
  ASSERT_TRUE(isRelativePath(P2("foo","bar")));
  ASSERT_TRUE(isRelativePath(P3(".","foo","bar")));
  ASSERT_TRUE(isRelativePath(P2("..","foo")));
  ASSERT_TRUE(isRelativePath(P4("foo","","bar","")));
  ASSERT_FALSE(isRelativePath(A1("foo")));
  ASSERT_FALSE(isRelativePath(A2("foo", "bar")));
}

TEST(pathTools, computeAbsolutePath)
{
  ASSERT_EQ(computeAbsolutePath(A2("a","b"), "foo"),
            A3("a","b","foo"));
  ASSERT_EQ(computeAbsolutePath(A3("a","b",""), "foo"),
            A3("a","b","foo"));
  ASSERT_EQ(computeAbsolutePath(A2("a","b"), P2(".","foo")),
            A3("a","b","foo"));
  ASSERT_EQ(computeAbsolutePath(A2("a","b"), P2("..","foo")),
            A2("a","foo"));
  ASSERT_EQ(computeAbsolutePath(A3("a","b",""), P2("..","foo")),
            A2("a","foo"));
  ASSERT_EQ(computeAbsolutePath(A5("a","b","c","d","e"), P2("..","foo")),
            A5("a","b","c","d","foo"));
  ASSERT_EQ(computeAbsolutePath(A5("a","b","c","d","e"), P5("..","..","..","g","foo")),
            A4("a","b","g","foo"));
}

TEST(pathTools, computeRelativePath)
{
  ASSERT_EQ(computeRelativePath(A2("a","b"), A3("a","b","foo")),
            "foo");
  ASSERT_EQ(computeRelativePath(A3("a","b",""), A3("a","b","foo")),
            "foo");
  ASSERT_EQ(computeRelativePath(A2("a","b"), A2("a","foo")),
            P2("..","foo"));
  ASSERT_EQ(computeRelativePath(A3("a","b",""), A2("a","foo")),
            P2("..","foo"));
  ASSERT_EQ(computeRelativePath(A5("a","b","c","d","e"), A5("a","b","c","d","foo")),
            P2("..","foo"));
  ASSERT_EQ(computeRelativePath(A5("a","b","c","d","e"), A4("a","b","g","foo")),
            P5("..","..","..","g","foo"));
}

TEST(pathTools, removeLastPathElement)
{
  ASSERT_EQ(removeLastPathElement(P3("a","b","c")),
            P2("a","b"));
  ASSERT_EQ(removeLastPathElement(A3("a","b","c")),
            A2("a","b"));
  ASSERT_EQ(removeLastPathElement(P4("a","b","c","")),
            P2("a","b"));
  ASSERT_EQ(removeLastPathElement(A4("a","b","c","")),
            A2("a","b"));
}

TEST(pathTools, appendToDirectory)
{
  ASSERT_EQ(appendToDirectory(P3("a","b","c"), "foo.xml"),
            P4("a","b","c","foo.xml"));
  ASSERT_EQ(appendToDirectory(P4("a","b","c",""), "foo.xml"),
            P4("a","b","c","foo.xml"));
  ASSERT_EQ(appendToDirectory(P3("a","b","c"), P2("d","foo.xml")),
            P5("a","b","c","d","foo.xml"));
  ASSERT_EQ(appendToDirectory(P4("a","b","c",""), P2("d","foo.xml")),
            P5("a","b","c","d","foo.xml"));
  ASSERT_EQ(appendToDirectory(P3("a","b","c"), P2(".","foo.xml")),
            P5("a","b","c",".","foo.xml"));
  ASSERT_EQ(appendToDirectory(P4("a","b","c",""), P2(".","foo.xml")),
            P5("a","b","c",".","foo.xml"));
}


TEST(pathTools, goUp)
{
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), ".."),
            A2("a", "b"));
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P2("..","..")),
            A1("a"));
#ifdef _WIN32
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P3("..","..","..")),
            "c:");
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P4("..","..","..","..")),
            "c:");
#else
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P3("..","..","..")),
            "/");
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P4("..","..","..","..")),
            "/");
#endif


  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P2("..", "foo")),
            A3("a", "b","foo"));
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P3("..","..","foo")),
            A2("a","foo"));
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P4("..","..","..","foo")),
            A1("foo"));
  ASSERT_EQ(computeAbsolutePath(A3("a","b","c"), P5("..","..","..","..","foo")),
            A1("foo"));
}


#ifdef _WIN32
TEST(pathTools, dirChange)
{
  std::string p1("c:\\a\\b\\c");
  std::string p2("d:\\d\\e\\foo.xml");
  std::string relative_path = computeRelativePath(p1, p2);
  ASSERT_EQ(relative_path, "d:\\d\\e\\foo.xml");
  std::string abs_path = computeAbsolutePath(p1, relative_path);
  ASSERT_EQ(abs_path, p2);
  ASSERT_EQ(computeAbsolutePath(p1, "..\\..\\..\\..\\..\\d:\\d\\e\\foo.xml"), p2);
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
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
