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
#include "../include/tools.h"
#include <string>
#include <vector>

namespace kiwix {
std::vector<std::string> split(const std::string&  base, const std::string& sep, bool trimEmpty, bool keepDelim);
};

using namespace kiwix;
#define parse_tag getTagValueFromTagList

namespace
{

// Some unit-tests may fail because of partial/missing ICU data. This test
// is intended to pinpoint to the root cause in such build environments.
TEST(stringTools, ICULanguageInfo)
{
  ASSERT_GE(ICULanguageInfo("en").selfName(),  "English");
  ASSERT_GE(ICULanguageInfo("eng").selfName(), "English");
  ASSERT_GE(ICULanguageInfo("fr").selfName(),  "français");
  ASSERT_GE(ICULanguageInfo("fra").selfName(), "français");
  ASSERT_GE(ICULanguageInfo("de").selfName(),  "Deutsch");
  ASSERT_GE(ICULanguageInfo("deu").selfName(), "Deutsch");
  ASSERT_GE(ICULanguageInfo("es").selfName(),  "español");
  ASSERT_GE(ICULanguageInfo("spa").selfName(), "español");
  ASSERT_GE(ICULanguageInfo("it").selfName(),  "italiano");
  ASSERT_GE(ICULanguageInfo("ita").selfName(), "italiano");
  ASSERT_GE(ICULanguageInfo("ru").selfName(),  "русский");
  ASSERT_GE(ICULanguageInfo("rus").selfName(), "русский");
  ASSERT_GE(ICULanguageInfo("hy").selfName(),  "հայերեն");
  ASSERT_GE(ICULanguageInfo("hye").selfName(), "հայերեն");
  ASSERT_GE(ICULanguageInfo("zh").selfName(),  "中文");
  ASSERT_GE(ICULanguageInfo("zho").selfName(), "中文");
  ASSERT_GE(ICULanguageInfo("ar").selfName(),  "العربية");
  ASSERT_GE(ICULanguageInfo("ara").selfName(), "العربية");
  ASSERT_GE(ICULanguageInfo("c++").selfName(), "c++");
}

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

namespace URLEncoding
{

const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char digits[] = "0123456789";
const char nonEncodableSymbols[] = ".-_~()*!/";
const char uriDelimSymbols[] = ":@?=+&#$;,";

const char otherSymbols[] = R"(`%^[]{}\|"<>)";

const char whitespace[] = " \n\t\r";

const char someNonASCIIChars[] = "Σ♂♀ツ";

}

TEST(stringTools, urlEncode)
{
  using namespace URLEncoding;

  EXPECT_EQ(urlEncode(letters), letters);

  EXPECT_EQ(urlEncode(digits), digits);

  EXPECT_EQ(urlEncode(nonEncodableSymbols), nonEncodableSymbols);

  EXPECT_EQ(urlEncode(uriDelimSymbols), "%3A%40%3F%3D%2B%26%23%24%3B%2C");

  EXPECT_EQ(urlEncode(otherSymbols), "%60%25%5E%5B%5D%7B%7D%5C%7C%22%3C%3E");

  EXPECT_EQ(urlEncode(whitespace), "%20%0A%09%0D");

  EXPECT_EQ(urlEncode(someNonASCIIChars), "%CE%A3%E2%99%82%E2%99%80%E3%83%84");
}

TEST(stringTools, urlDecode)
{
  using namespace URLEncoding;

  const std::string allTestChars = std::string(letters)
                                 + digits
                                 + nonEncodableSymbols
                                 + uriDelimSymbols
                                 + otherSymbols
                                 + whitespace
                                 + someNonASCIIChars;

  for ( const char c : allTestChars ) {
    const std::string str(1, c);
    EXPECT_EQ(urlDecode(urlEncode(str), true), str);
  }

  EXPECT_EQ(urlDecode(urlEncode(allTestChars), true), allTestChars);

  const std::string encodedUriDelimSymbols = urlEncode(uriDelimSymbols);
  EXPECT_EQ(urlDecode(encodedUriDelimSymbols, false), encodedUriDelimSymbols);
}

TEST(stringTools, stripSuffix)
{
  EXPECT_EQ(stripSuffix("abc123", "123"), "abc");
  EXPECT_EQ(stripSuffix("abc123", "123456789"), "abc123");
  EXPECT_EQ(stripSuffix("abc123", "987"), "abc123");
}

TEST(stringTools, getSlugifiedFileName)
{
  EXPECT_EQ(getSlugifiedFileName("abc123.png"), "abc123.png");
  EXPECT_EQ(getSlugifiedFileName("/"), "_");
  EXPECT_EQ(getSlugifiedFileName("abc/123.pdf"), "abc_123.pdf");
  EXPECT_EQ(getSlugifiedFileName("abc//123.yaml"), "abc__123.yaml");
  EXPECT_EQ(getSlugifiedFileName("//abc//123//"), "__abc__123__");
#ifdef _WIN32
  EXPECT_EQ(getSlugifiedFileName(R"(<>:"/\\|?*)"), "__________");
  EXPECT_EQ(getSlugifiedFileName(R"(<abc>:"/123\\|?*<.txt>)"), "_abc____123______.txt_");
#endif
}

};
