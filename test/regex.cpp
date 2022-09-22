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


#include "../src/tools/regexTools.h"

namespace
{

TEST(MatchRegex, match)
{
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "f"));
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "ef"));
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "efg"));
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "mno"));
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "m*o"));
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "m*v"));
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "M"));
  EXPECT_TRUE(matchRegex("abcdefghijklmnopqrstuvwxyz", "STU"));
  EXPECT_TRUE(matchRegex("Mythology & Folklore Stack Exchange", "folklore"));
  EXPECT_TRUE(matchRegex("Mythology &amp; Folklore Stack Exchange", "Folklore"));
  EXPECT_TRUE(matchRegex("Mythology &amp; Folklore Stack Exchange", "folklore"));
}

TEST(MatchRegex, nomatch)
{
  EXPECT_FALSE(matchRegex("abcdefghijklmnopqrstuvwxyz", "vu"));
}

TEST(ReplaceRegex, beginning)
{
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "abcd"), "----efghij");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "abcde"), "----fghij");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "a.*i"), "----j");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "AbCd"), "----efghij");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "A"), "----bcdefghij");
}

TEST(ReplaceRegex, end)
{
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "ghij"), "abcdef----");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "fghij"), "abcde----");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "c.*j"), "ab----");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "GhIj"), "abcdef----");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "J"), "abcdefghi----");
}

TEST(ReplaceRegex, middle)
{
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "cdef"), "ab----ghij");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "cdefgh"), "ab----ij");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "c.*f"), "ab----ghij");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "DeFg"), "abc----hij");
  EXPECT_EQ(replaceRegex("abcdefghij", "----", "F"), "abcde----ghij");
}

};
