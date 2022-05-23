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
#include "../src/tools/stringTools.h"
#include <string>
#include <vector>

namespace kiwix {
std::vector<std::string> split(const std::string&  base, const std::string& sep, bool trimEmpty, bool keepDelim);
};

using namespace kiwix;
#define parse_tag getTagValueFromTagList

namespace
{
TEST(stringTools, join)
{
  std::vector<std::string> list = { "a", "b", "c" };
  ASSERT_EQ(join(list, ";"), "a;b;c");
}

TEST(stringTools, split)
{
  std::vector<std::string> list1 = { "a", "b", "c" };
  ASSERT_EQ(split("a;b;c", ";", false, false), list1);
  ASSERT_EQ(split("a;b;c", ";", true, false), list1);
  std::vector<std::string> list2 = { "", "a", "b", "c" };
  ASSERT_EQ(split(";a;b;c", ";", false, false), list2);
  ASSERT_EQ(split(";a;b;c", ";", true, false), list1);
  std::vector<std::string> list3 = { "", "a", "b", "c", ""};
  ASSERT_EQ(split(";a;b;c;", ";", false, false), list3);
  ASSERT_EQ(split(";a;b;c;", ";", true, false), list1);
  std::vector<std::string> list4 = { "", "a", "b", "", "c", ""};
  ASSERT_EQ(split(";a;b;;c;", ";", false, false), list4);
  ASSERT_EQ(split(";a;b;;c;", ";", true, false), list1);

  std::vector<std::string> list5 = { ";", "a", ";", "b", "=", ";", "c", "=", "d", ";"};
  ASSERT_EQ(split(";a;b=;c=d;", ";=", true, true), list5);
  std::vector<std::string> list6 = { "", ";", "a", ";", "b", "=", "", ";", "c", "=", "d", ";", ""};
  ASSERT_EQ(split(";a;b=;c=d;", ";=", false, true), list6);
}

TEST(stringTools, extractFromString)
{
  ASSERT_EQ(extractFromString<int>("55"), 55);
  ASSERT_EQ(extractFromString<int>("-55"), -55);
  ASSERT_EQ(extractFromString<float>("-55.0"), -55.0);
  ASSERT_EQ(extractFromString<bool>("1"), true);
  ASSERT_EQ(extractFromString<bool>("0"), false);
  ASSERT_EQ(extractFromString<std::string>("55"), "55");
  ASSERT_EQ(extractFromString<std::string>("foo"), "foo");
  ASSERT_EQ(extractFromString<std::string>("foo bar"), "foo bar");

// While spec says that >> operator should set the value to std::numeric_limits<uint>::max()
// and set the failbit of the stream (and so detect the error), the gcc implementation (?)
// set the value to (std::numeric_limits<uint>::max()-55) and doesn't set the failbit.
//  ASSERT_THROW(extractFromString<uint>("-55"), std::invalid_argument);

  ASSERT_THROW(extractFromString<int>("-55.0"), std::invalid_argument);
  ASSERT_THROW(extractFromString<int>("55 foo"), std::invalid_argument);
  ASSERT_THROW(extractFromString<float>("3.14.5"), std::invalid_argument);
}

};
