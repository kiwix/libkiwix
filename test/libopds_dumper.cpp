/*
 * Copyright 2026 Hamazasp Avetisyan <hamik.avetisyan@gmail.com>
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

#include "../include/library.h"
#include "../include/book.h"
#include "../src/libopds_dumper.h"

#include <pugixml.hpp>

namespace
{

using namespace kiwix;

Book createBook(const std::string& id, const std::string& title)
{
  Book book;
  book.setId(id);
  book.setPath("/local/path/" + id + ".zim");
  book.setTitle(title);
  book.setDescription("Description of " + title);
  book.setLanguage("eng");
  book.setCreator(title + " Creator");
  book.setPublisher(title + " Publisher");
  book.setDate("2021-03-25");
  book.setUrl("http://download.kiwix.org/zim/" + id + ".zim");
  book.setName("test_" + id);
  book.setTags("tag1;tag2;_category:test");
  book.setArticleCount(42);
  book.setMediaCount(7);
  book.setSize(123456);
  return book;
}

// Parses `opdsContent` as XML and returns the resulting document. Fails the
// current test if the content is not well-formed XML (e.g. due to improperly
// escaped book metadata).
pugi::xml_document parseOPDS(const std::string& opdsContent)
{
  pugi::xml_document doc;
  const auto result = doc.load_string(opdsContent.c_str());
  EXPECT_TRUE(result) << "OPDS content is not valid XML: " << result.description()
                       << "\n" << opdsContent;
  return doc;
}

class LibOPDSDumperTest : public ::testing::Test {
 protected:
  void SetUp() override {
    lib = Library::create();
    lib->addBook(createBook("book1-id", "First Book"));
    lib->addBook(createBook("book2-id", "Second Book"));

    dumper.setLibrary(lib.get());
    dumper.setRootLocation("http://root.location");
  }

  LibraryPtr lib;
  LibOPDSDumper dumper;
};

// NB: the macro parameter names must not collide with any token used in the
// body (e.g. the pugixml member names "node"/"text"), since preprocessor
// substitution is purely textual and would otherwise mangle those calls.
#define EXPECT_XPATH_TEXT(xmlNode, xpathExpr, expectedText) \
  EXPECT_EQ(std::string(expectedText), std::string((xmlNode).select_node(xpathExpr).node().text().as_string()))

TEST_F(LibOPDSDumperTest, emptyBookIdListProducesEmptyFeed)
{
  const std::string opdsContent = dumper.dumpOPDSContent({});
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  ASSERT_TRUE(feed);
  EXPECT_EQ(0, std::distance(feed.children("entry").begin(), feed.children("entry").end()));
}

TEST_F(LibOPDSDumperTest, dumpsRequestedBooksOnly)
{
  const std::string opdsContent = dumper.dumpOPDSContent({"book1-id"});
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  const auto entries = feed.children("entry");
  EXPECT_EQ(1, std::distance(entries.begin(), entries.end()));

  const auto entry = feed.child("entry");
  EXPECT_EQ("urn:uuid:book1-id", std::string(entry.child_value("id")));
  EXPECT_XPATH_TEXT(entry, "title", "First Book");
  EXPECT_XPATH_TEXT(entry, "summary", "Description of First Book");
  EXPECT_XPATH_TEXT(entry, "language", "eng");
  EXPECT_XPATH_TEXT(entry, "name", "test_book1-id");
  EXPECT_XPATH_TEXT(entry, "tags", "tag1;tag2;_category:test");
  EXPECT_XPATH_TEXT(entry, "articleCount", "42");
  EXPECT_XPATH_TEXT(entry, "mediaCount", "7");
  EXPECT_XPATH_TEXT(entry, "author/name", "First Book Creator");
  EXPECT_XPATH_TEXT(entry, "publisher/name", "First Book Publisher");

  const auto acquisitionLink = entry.select_node(
      "link[@rel='http://opds-spec.org/acquisition/open-access']").node();
  ASSERT_TRUE(acquisitionLink);
  EXPECT_STREQ("http://download.kiwix.org/zim/book1-id.zim", acquisitionLink.attribute("href").as_string());
  EXPECT_STREQ("123456", acquisitionLink.attribute("length").as_string());
}

TEST_F(LibOPDSDumperTest, dumpsAllRequestedBooksInOrder)
{
  const std::string opdsContent = dumper.dumpOPDSContent({"book2-id", "book1-id"});
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  const auto entries = feed.children("entry");
  ASSERT_EQ(2, std::distance(entries.begin(), entries.end()));

  auto it = entries.begin();
  EXPECT_EQ("urn:uuid:book2-id", std::string(it->child_value("id")));
  ++it;
  EXPECT_EQ("urn:uuid:book1-id", std::string(it->child_value("id")));
}

TEST_F(LibOPDSDumperTest, unknownBookIdsAreSilentlyIgnored)
{
  const std::string opdsContent = dumper.dumpOPDSContent({"book1-id", "no-such-book", "book2-id"});
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  const auto entries = feed.children("entry");
  EXPECT_EQ(2, std::distance(entries.begin(), entries.end()));
}

TEST_F(LibOPDSDumperTest, defaultConstructedDumperHasNoLibrary)
{
  LibOPDSDumper freshDumper;
  const std::string opdsContent = freshDumper.dumpOPDSContent({"book1-id"});
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  ASSERT_TRUE(feed);
  const auto entries = feed.children("entry");
  EXPECT_EQ(0, std::distance(entries.begin(), entries.end()));
}

TEST_F(LibOPDSDumperTest, setLibraryReplacesTheDumpedLibrary)
{
  auto otherLib = Library::create();
  otherLib->addBook(createBook("other-book-id", "Other Book"));

  dumper.setLibrary(otherLib.get());

  const std::string opdsContent = dumper.dumpOPDSContent({"book1-id", "other-book-id"});
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  const auto entries = feed.children("entry");
  // "book1-id" belongs to the previously set library and is no longer known.
  ASSERT_EQ(1, std::distance(entries.begin(), entries.end()));
  EXPECT_EQ("urn:uuid:other-book-id", std::string(feed.child("entry").child_value("id")));
}

TEST_F(LibOPDSDumperTest, rootLocationIsUsedForFeedIdAndSearchLink)
{
  const std::string opdsContent = dumper.dumpOPDSContent({});
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  EXPECT_STRNE("", feed.child_value("id"));

  const auto searchLink = feed.select_node("link[@rel='search']").node();
  ASSERT_TRUE(searchLink);
  EXPECT_STREQ("http://root.location/catalog/searchdescription.xml",
               searchLink.attribute("href").as_string());
}

TEST_F(LibOPDSDumperTest, specialCharactersInBookMetadataAreEscaped)
{
  Book book = createBook("special-id", "Q&A <Tricky> \"Title\"");
  book.setDescription("Ideas & \"quotes\" <tags>");
  lib->addBook(book);

  const std::string opdsContent = dumper.dumpOPDSContent({"special-id"});
  // parseOPDS() itself fails the test if the XML is not well-formed, which
  // would happen if '&'/'<'/'"' were not properly escaped.
  const auto doc = parseOPDS(opdsContent);

  const auto entry = doc.select_node("//entry").node();
  EXPECT_XPATH_TEXT(entry, "title", "Q&A <Tricky> \"Title\"");
  EXPECT_XPATH_TEXT(entry, "summary", "Ideas & \"quotes\" <tags>");
}

#undef EXPECT_XPATH_TEXT

} // unnamed namespace
