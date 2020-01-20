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

namespace kiwix {
std::string join(const std::vector<std::string>& list, const std::string& sep);
std::vector<std::string> split(const std::string&  base, const std::string& sep, bool trimEmpty);
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
  ASSERT_EQ(split("a;b;c", ";", false), list1);
  ASSERT_EQ(split("a;b;c", ";", true), list1);
  std::vector<std::string> list2 = { "", "a", "b", "c" };
  ASSERT_EQ(split(";a;b;c", ";", false), list2);
  ASSERT_EQ(split(";a;b;c", ";", true), list1);
  std::vector<std::string> list3 = { "", "a", "b", "c", ""};
  ASSERT_EQ(split(";a;b;c;", ";", false), list3);
  ASSERT_EQ(split(";a;b;c;", ";", true), list1);
  std::vector<std::string> list4 = { "", "a", "b", "", "c", ""};
  ASSERT_EQ(split(";a;b;;c;", ";", false), list4);
  ASSERT_EQ(split(";a;b;;c;", ";", true), list1);
}

};
