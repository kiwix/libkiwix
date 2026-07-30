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

#include "../include/book.h"
#include "../include/library.h"
#include "../include/manager.h"
#include "../src/library_dumper.h"

#include <pugixml.hpp>

namespace
{

using namespace kiwix;

Book createBook()
{
  Book book;
  book.setId("book-id");
  book.setTitle("Some Title");
  book.setDescription("Some Description");
  book.setLanguage("eng");
  book.setCreator("Some Creator");
  book.setPublisher("Some Publisher");
  book.setDate("2021-03-25");
  book.setName("some_name");
  book.setTags("tag1;tag2;_category:wikipedia");
  book.setFlavour("nopic");
  book.setArticleCount(42);
  book.setMediaCount(7);
  return book;
}

// Parses `entryXml` as XML and returns the resulting document. Fails the
// current test if the content is not well-formed XML (e.g. due to
// improperly escaped book metadata).
pugi::xml_document parseEntry(const std::string& entryXml)
{
  pugi::xml_document doc;
  const auto result = doc.load_string(entryXml.c_str());
  EXPECT_TRUE(result) << "entry XML is not well-formed: " << result.description()
                       << "\n" << entryXml;
  return doc;
}

// NB: the macro parameter names must not collide with any token used in the
// body (e.g. the pugixml member names "node"/"text"), since preprocessor
// substitution is purely textual and would otherwise mangle those calls.
#define EXPECT_XPATH_TEXT(xmlNode, xpathExpr, expectedText) \
  EXPECT_EQ(std::string(expectedText), std::string((xmlNode).select_node(xpathExpr).node().text().as_string()))

TEST(FullEntryOpdsTest, rendersCoreBookFields)
{
  const Book book = createBook();
  const auto entryXml = fullEntryOpds(book, "http://root.location", /*contentAccessUrl=*/"", /*contentId=*/"book-id");
  const auto doc = parseEntry(entryXml);

  const auto entry = doc.select_node("/entry").node();
  ASSERT_TRUE(entry);
  EXPECT_EQ("urn:uuid:book-id", std::string(entry.child_value("id")));
  EXPECT_XPATH_TEXT(entry, "title", "Some Title");
  EXPECT_XPATH_TEXT(entry, "summary", "Some Description");
  EXPECT_XPATH_TEXT(entry, "language", "eng");
  EXPECT_XPATH_TEXT(entry, "name", "some_name");
  EXPECT_XPATH_TEXT(entry, "flavour", "nopic");
  // Book::getCategory() only reflects a "_category:" tag/attribute resolved
  // during XML/OPDS parsing - it is not derived on the fly from setTags(),
  // so a manually-constructed Book always reports an empty category here.
  EXPECT_XPATH_TEXT(entry, "category", "");
  EXPECT_XPATH_TEXT(entry, "tags", "tag1;tag2;_category:wikipedia");
  EXPECT_XPATH_TEXT(entry, "articleCount", "42");
  EXPECT_XPATH_TEXT(entry, "mediaCount", "7");
  EXPECT_XPATH_TEXT(entry, "author/name", "Some Creator");
  EXPECT_XPATH_TEXT(entry, "publisher/name", "Some Publisher");
  EXPECT_XPATH_TEXT(entry, "dc:issued", "2021-03-25T00:00:00Z");
  EXPECT_XPATH_TEXT(entry, "updated", "2021-03-25T00:00:00Z");
}

TEST(FullEntryOpdsTest, omitsAcquisitionLinkWhenUrlIsEmpty)
{
  const Book book = createBook();
  const auto entryXml = fullEntryOpds(book, "http://root.location", "", "book-id");
  const auto doc = parseEntry(entryXml);

  const auto entry = doc.select_node("/entry").node();
  EXPECT_FALSE(entry.select_node("link[@rel='http://opds-spec.org/acquisition/open-access']"));
}

TEST(FullEntryOpdsTest, rendersAcquisitionLinkWhenUrlIsSet)
{
  Book book = createBook();
  book.setUrl("http://download.kiwix.org/zim/book.zim");
  book.setSize(123456);

  const auto entryXml = fullEntryOpds(book, "http://root.location", "", "book-id");
  const auto doc = parseEntry(entryXml);

  const auto entry = doc.select_node("/entry").node();
  const auto link = entry.select_node("link[@rel='http://opds-spec.org/acquisition/open-access']").node();
  ASSERT_TRUE(link);
  EXPECT_STREQ("http://download.kiwix.org/zim/book.zim", link.attribute("href").as_string());
  EXPECT_STREQ("123456", link.attribute("length").as_string());
  EXPECT_STREQ("application/x-zim", link.attribute("type").as_string());
}

TEST(FullEntryOpdsTest, omitsContentLinkWhenContentAccessUrlIsEmpty)
{
  const Book book = createBook();
  const auto entryXml = fullEntryOpds(book, "http://root.location", /*contentAccessUrl=*/"", "book-id");
  const auto doc = parseEntry(entryXml);

  const auto entry = doc.select_node("/entry").node();
  EXPECT_FALSE(entry.select_node("link[@type='text/html']"));
}

TEST(FullEntryOpdsTest, rendersUrlEncodedContentLinkWhenContentAccessUrlIsSet)
{
  const Book book = createBook();
  // '/' is one of urlEncode()'s "harmless" characters (paths may legitimately
  // contain it) and passes through untouched, while ' ' and '#' do not.
  const auto entryXml = fullEntryOpds(book, "http://root.location", "http://root.location/content", "a/b c#d");
  const auto doc = parseEntry(entryXml);

  const auto entry = doc.select_node("/entry").node();
  const auto link = entry.select_node("link[@type='text/html']").node();
  ASSERT_TRUE(link);
  EXPECT_STREQ("http://root.location/content/a/b%20c%23d", link.attribute("href").as_string());
}

TEST(FullEntryOpdsTest, rendersThumbnailLinkForBookIllustration)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleXML[] = R"(
<library version="1.0">
  <book
        id="book-with-icon"
        path="/local/path/book.zim"
        title="Book With Icon"
        favicon="AAAA"
        faviconMimeType="image/png"
        faviconUrl="/favicon.png"
      ></book>
</library>
)";
  manager.readXml(sampleXML, /*readOnly=*/false, "", /*trustLibrary=*/true);
  const Book& book = lib->getBookById("book-with-icon");

  const auto entryXml = fullEntryOpds(book, "http://root.location", "", "book-with-icon");
  const auto doc = parseEntry(entryXml);

  const auto entry = doc.select_node("/entry").node();
  const auto link = entry.select_node("link[@rel='http://opds-spec.org/image/thumbnail']").node();
  ASSERT_TRUE(link);
  EXPECT_STREQ("http://root.location/catalog/v2/illustration/book-with-icon/?size=48",
               link.attribute("href").as_string());
  EXPECT_STREQ("image/png;width=48;height=48;scale=1", link.attribute("type").as_string());
}

TEST(FullEntryOpdsTest, specialCharactersInBookMetadataAreEscaped)
{
  Book book = createBook();
  book.setTitle("Q&A <Tricky> \"Title\"");
  book.setDescription("Ideas & \"quotes\" <tags>");

  const auto entryXml = fullEntryOpds(book, "http://root.location", "", "book-id");
  // parseEntry() itself fails the test if the XML is not well-formed, which
  // would happen if '&'/'<'/'"' were not properly escaped.
  const auto doc = parseEntry(entryXml);

  const auto entry = doc.select_node("/entry").node();
  EXPECT_XPATH_TEXT(entry, "title", "Q&A <Tricky> \"Title\"");
  EXPECT_XPATH_TEXT(entry, "summary", "Ideas & \"quotes\" <tags>");
}

#undef EXPECT_XPATH_TEXT

} // unnamed namespace
