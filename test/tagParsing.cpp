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
std::vector<std::string> convertTags(const std::string& tags);
std::string getTagValueFromTagList(const std::vector<std::string>& tagList, const std::string& tagName);
};

using namespace kiwix;
#define parse_tag getTagValueFromTagList

namespace
{
TEST(ParseTagTest, convert)
{
  {
    std::string tagStr = "";
    std::vector<std::string> tagList = {"_ftindex:no", "_pictures:yes", "_videos:yes", "_details:yes"};
    ASSERT_EQ(convertTags(tagStr), tagList);
  }
  {
    std::string tagStr = "_category:foo;bar";
    std::vector<std::string> tagList = {"_category:foo", "bar", "_ftindex:no", "_pictures:yes", "_videos:yes", "_details:yes"};
    ASSERT_EQ(convertTags(tagStr), tagList);
  }
  {
    std::string tagStr = "_ftindex:no;_pictures:yes;_videos:yes;_details:yes;_category:foo;bar";
    std::vector<std::string> tagList = {"_ftindex:no", "_pictures:yes", "_videos:yes", "_details:yes", "_category:foo", "bar"};
    ASSERT_EQ(convertTags(tagStr), tagList);
  }
  {
    std::string tagStr = "_ftindex:yes;_pictures:no;_videos:no;_details:no;_category:foo;bar";
    std::vector<std::string> tagList = {"_ftindex:yes", "_pictures:no", "_videos:no", "_details:no", "_category:foo", "bar"};
    ASSERT_EQ(convertTags(tagStr), tagList);
  }
  {
    std::string tagStr = "_ftindex;nopic;novid;nodet;foo;bar";
    std::vector<std::string> tagList = {"_ftindex:yes", "_pictures:no", "_videos:no", "_details:no", "foo", "bar"};
    ASSERT_EQ(convertTags(tagStr), tagList);
  }
}

TEST(ParseTagTest, valid)
{
  std::string tagStr = "_ftindex:yes;_pictures:no;_videos:no;_details:yes;_category:foo;bar";
  auto tagList = convertTags(tagStr);

  ASSERT_EQ(parse_tag(tagList, "ftindex"), "yes");
  ASSERT_EQ(parse_tag(tagList, "pictures"), "no");
  ASSERT_EQ(parse_tag(tagList, "category"), "foo");
  ASSERT_EQ(parse_tag(tagList, "details"), "yes");
  ASSERT_THROW(parse_tag(tagList, "detail"), std::out_of_range);
}

TEST(ParseTagTest, compat)
{
  std::string tagStr = "_ftindex;nopic;foo;bar";
  auto tagList = convertTags(tagStr);

  ASSERT_EQ(parse_tag(tagList, "ftindex"), "yes");
  ASSERT_EQ(parse_tag(tagList, "pictures"), "no");
  ASSERT_EQ(parse_tag(tagList, "videos"), "yes");
  ASSERT_EQ(parse_tag(tagList, "details"), "yes");
}

TEST(ParseTagTest, invalid)
{
  std::string tagStr = "_ftindex:y;_pictures;_videos:;_details:yes;_details:no;_category:foo;bar";
  auto tagList = convertTags(tagStr);

  ASSERT_EQ(parse_tag(tagList, "ftindex"), "y");
  ASSERT_EQ(parse_tag(tagList, "pictures"), "yes");
  ASSERT_EQ(parse_tag(tagList, "videos"), "");
  ASSERT_EQ(parse_tag(tagList, "details"), "yes");
}

};
