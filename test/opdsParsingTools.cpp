/*
 * Copyright (C) 2023 Nikhil Tanwar (2002nikhiltanwar@gmail.com)
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
#include "../include/tools.h"
typedef kiwix::FeedLanguages FeedLanguages;
typedef kiwix::FeedCategories FeedCategories;

namespace
{
const char sampleLanguageOpdsStream[] = R"(
<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:dc="http://purl.org/dc/terms/"
      xmlns:opds="https://specs.opds.io/opds-1.2"
      xmlns:thr="http://purl.org/syndication/thread/1.0">
  <id>1e587935-0f7b-dad6-eddc-ef3fafd4c3ed</id>
  <link rel="self"
        href="/catalog/v2/languages"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <title>List of languages</title>
  <updated>2023-07-11T15:35:09Z</updated>

  <entry>
    <title>Abkhazian</title>
    <dc:language>abk</dc:language>
    <thr:count>3</thr:count>
    <link rel="subsection"
          href="/catalog/v2/entries?lang=abk"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>2023-07-11T15:35:09Z</updated>
    <id>2e4d9a1c-9750-0418-8124-a0c663e206f7</id>
  </entry>
  <entry>
    <title>isiZulu</title>
    <dc:language>zul</dc:language>
    <thr:count>4</thr:count>
    <link rel="subsection"
          href="/catalog/v2/entries?lang=zul"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>2023-07-11T15:35:09Z</updated>
    <id>76eec223-994d-9b95-e309-baee06e585b0</id>
  </entry>
</feed>
)";

const char sampleCategoriesOpdsStream[] = R"(
<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <id>231da20c-0fe0-7345-11b2-d29f50364108</id>
  <link rel="self"
        href="/catalog/v2/categories"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <title>List of categories</title>
  <updated>2023-07-11T15:35:09Z</updated>

  <entry>
    <title>gutenberg</title>
    <link rel="subsection"
          href="/catalog/v2/entries?category=gutenberg"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>2023-07-11T15:35:09Z</updated>
    <id>401dbe68-2f7a-5503-b431-054801c30bab</id>
    <content type="text">All entries with category of 'gutenberg'.</content>
  </entry>
  <entry>
    <title>iFixit</title>
    <link rel="subsection"
          href="/catalog/v2/entries?category=iFixit"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>2023-07-11T15:35:09Z</updated>
    <id>c18e5459-af23-5fbf-0622-ff271bd9a5ad</id>
    <content type="text">All entries with category of 'iFixit'.</content>
  </entry>
  <entry>
    <title>wikivoyage</title>
    <link rel="subsection"
          href="/catalog/v2/entries?category=wikivoyage"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>2023-07-11T15:35:09Z</updated>
    <id>9a75be6c-7a35-6f52-1a69-bee9ad248459</id>
    <content type="text">All entries with category of 'wikivoyage'.</content>
  </entry>
  <entry>
    <title>wiktionary</title>
    <link rel="subsection"
          href="/catalog/v2/entries?category=wiktionary"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>2023-07-11T15:35:09Z</updated>
    <id>7adb9f1a-73d7-0391-1238-d2e2c300ddaa</id>
    <content type="text">All entries with category of 'wiktionary'.</content>
  </entry>
</feed>
)";

TEST(OpdsParsingTest, languageTest)
{
    FeedLanguages expectedLanguagesFromFeed = {{"abk", "Abkhazian"}, {"zul", "isiZulu"}};
    EXPECT_EQ(kiwix::readLanguagesFromFeed(sampleLanguageOpdsStream), expectedLanguagesFromFeed);
}

TEST(OpdsParsingTest, categoryTest)
{
    FeedCategories expectedCategoriesFromFeed = {"gutenberg", "iFixit", "wikivoyage", "wiktionary"};
    EXPECT_EQ(kiwix::readCategoriesFromFeed(sampleCategoriesOpdsStream), expectedCategoriesFromFeed);
}

}
