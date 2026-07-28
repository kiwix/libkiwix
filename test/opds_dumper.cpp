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
#include "../include/manager.h"
#include "../include/name_mapper.h"
#include "../include/book.h"
#include "../include/tools.h"
#include "../src/opds_dumper.h"
#include "../src/tools/stringTools.h"
#include "../src/tools/otherTools.h"

#include <pugixml.hpp>

namespace
{

using namespace kiwix;

// Two books, deliberately exercising the two different ways a category can
// be attributed to a book (an explicit "category" attribute vs. a
// "_category:" tag), and two different languages.
const char sampleLibraryXML[] = R"(
<library version="1.0">
  <book
        id="book1-id"
        path="/local/path/book1-id.zim"
        title="First Book"
        description="Description of First Book"
        language="eng"
        creator="First Book Creator"
        publisher="First Book Publisher"
        date="2021-03-25"
        url="http://download.kiwix.org/zim/book1-id.zim"
        name="test_book1-id"
        tags="tag1;tag2;_category:wikipedia"
        articleCount="42"
        mediaCount="7"
        size="120"
      ></book>
  <book
        id="book2-id"
        path="/local/path/book2-id.zim"
        title="Second Book"
        description="Description of Second Book"
        language="fra"
        creator="Second Book Creator"
        publisher="Second Book Publisher"
        date="2021-03-26"
        url="http://download.kiwix.org/zim/book2-id.zim"
        name="test_book2-id"
        category="wiktionary"
        tags="tag3"
        articleCount="10"
        mediaCount="1"
        size="45"
      ></book>
</library>
)";

// Parses `opdsContent` as XML and returns the resulting document. Fails the
// current test if the content is not well-formed XML.
pugi::xml_document parseOPDS(const std::string& opdsContent)
{
  pugi::xml_document doc;
  const auto result = doc.load_string(opdsContent.c_str());
  EXPECT_TRUE(result) << "OPDS content is not valid XML: " << result.description()
                       << "\n" << opdsContent;
  return doc;
}

// NB: the macro parameter names must not collide with any token used in the
// body (e.g. the pugixml member names "node"/"text"), since preprocessor
// substitution is purely textual and would otherwise mangle those calls.
#define EXPECT_XPATH_TEXT(xmlNode, xpathExpr, expectedText) \
  EXPECT_EQ(std::string(expectedText), std::string((xmlNode).select_node(xpathExpr).node().text().as_string()))

class OPDSDumperTest : public ::testing::Test {
 protected:
  void SetUp() override {
    lib = Library::create();
    Manager manager(lib);
    manager.readXml(sampleLibraryXML, /*readOnly=*/false, "", /*trustLibrary=*/true);

    dumper.reset(new OPDSDumper(lib.get(), &nameMapper));
    dumper->setRootLocation("http://root.location");
    dumper->setLibraryId("test-library-id");
    dumper->setContentAccessUrl("http://root.location/content");
    dumper->setOpenSearchInfo(/*totalResults=*/2, /*startIndex=*/0, /*count=*/1);
  }

  LibraryPtr lib;
  IdNameMapper nameMapper;
  std::unique_ptr<OPDSDumper> dumper;
};

TEST_F(OPDSDumperTest, dumpOPDSFeedWithoutQueryProducesUnfilteredFeed)
{
  const std::string opdsContent = dumper->dumpOPDSFeed({"book1-id"}, "");
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  ASSERT_TRUE(feed);
  EXPECT_XPATH_TEXT(feed, "title", "All zims");
  EXPECT_FALSE(feed.child("totalResults"));

  const auto searchLink = feed.select_node("link[@rel='search']").node();
  ASSERT_TRUE(searchLink);
  EXPECT_STREQ("http://root.location/catalog/searchdescription.xml",
               searchLink.attribute("href").as_string());

  const auto entries = feed.children("entry");
  ASSERT_EQ(1, std::distance(entries.begin(), entries.end()));

  const auto entry = feed.child("entry");
  EXPECT_EQ("urn:uuid:book1-id", std::string(entry.child_value("id")));
  EXPECT_XPATH_TEXT(entry, "title", "First Book");
  EXPECT_XPATH_TEXT(entry, "name", "test_book1-id");

  // IdNameMapper maps the book id to itself, so the content link uses the
  // book id verbatim.
  const auto contentLink = entry.select_node("link[@type='text/html']").node();
  ASSERT_TRUE(contentLink);
  EXPECT_STREQ("http://root.location/content/book1-id", contentLink.attribute("href").as_string());
}

TEST_F(OPDSDumperTest, dumpOPDSFeedWithQueryIncludesFilterAndSearchInfo)
{
  const std::string opdsContent = dumper->dumpOPDSFeed({"book1-id"}, "lang=eng");
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  EXPECT_XPATH_TEXT(feed, "title", "Filtered zims (lang=eng)");
  EXPECT_XPATH_TEXT(feed, "totalResults", "2");
  EXPECT_XPATH_TEXT(feed, "startIndex", "0");
  EXPECT_XPATH_TEXT(feed, "itemsPerPage", "1");
}

TEST_F(OPDSDumperTest, dumpOPDSFeedV2ProducesFullEntriesByDefault)
{
  const std::string opdsContent = dumper->dumpOPDSFeedV2({"book1-id", "book2-id"}, "", /*partial=*/false);
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  ASSERT_TRUE(feed);
  EXPECT_XPATH_TEXT(feed, "title", "All Entries");

  const auto selfLink = feed.select_node("link[@rel='self']").node();
  ASSERT_TRUE(selfLink);
  EXPECT_STREQ("http://root.location/catalog/v2/entries", selfLink.attribute("href").as_string());

  const auto entries = feed.children("entry");
  ASSERT_EQ(2, std::distance(entries.begin(), entries.end()));

  const auto entry = feed.child("entry");
  EXPECT_XPATH_TEXT(entry, "title", "First Book");
  EXPECT_XPATH_TEXT(entry, "summary", "Description of First Book");
  EXPECT_XPATH_TEXT(entry, "articleCount", "42");
  EXPECT_XPATH_TEXT(entry, "mediaCount", "7");

  const auto acquisitionLink = entry.select_node(
      "link[@rel='http://opds-spec.org/acquisition/open-access']").node();
  ASSERT_TRUE(acquisitionLink);
  EXPECT_STREQ("http://download.kiwix.org/zim/book1-id.zim", acquisitionLink.attribute("href").as_string());
  EXPECT_STREQ(std::to_string(120UL << 10).c_str(), acquisitionLink.attribute("length").as_string());
}

TEST_F(OPDSDumperTest, dumpOPDSFeedV2PartialProducesMinimalEntries)
{
  const std::string opdsContent = dumper->dumpOPDSFeedV2({"book1-id"}, "", /*partial=*/true);
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  const auto selfLink = feed.select_node("link[@rel='self']").node();
  ASSERT_TRUE(selfLink);
  EXPECT_STREQ("http://root.location/catalog/v2/partial_entries", selfLink.attribute("href").as_string());

  const auto entry = feed.child("entry");
  ASSERT_TRUE(entry);
  EXPECT_EQ("urn:uuid:book1-id", std::string(entry.child_value("id")));
  EXPECT_XPATH_TEXT(entry, "title", "First Book");
  // Partial entries carry only id/title/updated - no summary, tags, etc.
  EXPECT_FALSE(entry.child("summary"));
  EXPECT_FALSE(entry.child("tags"));

  const auto altLink = entry.select_node("link[@rel='alternate']").node();
  ASSERT_TRUE(altLink);
  EXPECT_STREQ("http://root.location/catalog/v2/entry/book1-id", altLink.attribute("href").as_string());
}

TEST_F(OPDSDumperTest, dumpOPDSCompleteEntryProducesStandaloneEntryDocument)
{
  const std::string opdsContent = dumper->dumpOPDSCompleteEntry("book2-id");

  EXPECT_EQ(0U, opdsContent.rfind("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 0));

  const auto doc = parseOPDS(opdsContent);
  const auto entry = doc.select_node("/entry").node();
  ASSERT_TRUE(entry);
  EXPECT_EQ("urn:uuid:book2-id", std::string(entry.child_value("id")));
  EXPECT_XPATH_TEXT(entry, "title", "Second Book");
  EXPECT_XPATH_TEXT(entry, "name", "test_book2-id");
}

TEST_F(OPDSDumperTest, dumpOPDSCompleteEntryThrowsForUnknownBookId)
{
  EXPECT_THROW(dumper->dumpOPDSCompleteEntry("no-such-book"), std::out_of_range);
}

TEST_F(OPDSDumperTest, categoriesOPDSFeedListsDistinctSortedCategories)
{
  const std::string opdsContent = dumper->categoriesOPDSFeed();
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  ASSERT_TRUE(feed);
  EXPECT_XPATH_TEXT(feed, "title", "List of categories");

  const auto entries = feed.children("entry");
  ASSERT_EQ(2, std::distance(entries.begin(), entries.end()));

  // Categories are listed in sorted order: "wikipedia" (from the
  // "_category:" tag on book1) before "wiktionary" (from book2's explicit
  // "category" attribute).
  auto it = entries.begin();
  EXPECT_XPATH_TEXT(*it, "title", "wikipedia");
  EXPECT_XPATH_TEXT(*it, "content", "All entries with category of 'wikipedia'.");
  const auto wikipediaLink = it->select_node("link[@rel='subsection']").node();
  ASSERT_TRUE(wikipediaLink);
  EXPECT_STREQ("http://root.location/catalog/v2/entries?category=wikipedia",
               wikipediaLink.attribute("href").as_string());
  EXPECT_EQ(gen_uuid("test-library-id/categories/wikipedia"), std::string(it->child_value("id")));

  ++it;
  EXPECT_XPATH_TEXT(*it, "title", "wiktionary");
}

TEST_F(OPDSDumperTest, languagesOPDSFeedListsLanguagesWithCounts)
{
  const std::string opdsContent = dumper->languagesOPDSFeed();
  const auto doc = parseOPDS(opdsContent);

  const auto feed = doc.select_node("/feed").node();
  ASSERT_TRUE(feed);
  EXPECT_XPATH_TEXT(feed, "title", "List of languages");

  const auto entries = feed.children("entry");
  ASSERT_EQ(2, std::distance(entries.begin(), entries.end()));

  // Languages are listed in sorted order of their ISO code: "eng" before "fra".
  auto it = entries.begin();
  EXPECT_XPATH_TEXT(*it, "title", getLanguageSelfName("eng"));
  EXPECT_XPATH_TEXT(*it, "dc:language", "eng");
  EXPECT_XPATH_TEXT(*it, "thr:count", "1");
  const auto engLink = it->select_node("link[@rel='subsection']").node();
  ASSERT_TRUE(engLink);
  EXPECT_STREQ("http://root.location/catalog/v2/entries?lang=eng", engLink.attribute("href").as_string());
  EXPECT_EQ(gen_uuid("test-library-id/languages/eng"), std::string(it->child_value("id")));

  ++it;
  EXPECT_XPATH_TEXT(*it, "title", getLanguageSelfName("fra"));
  EXPECT_XPATH_TEXT(*it, "dc:language", "fra");
  EXPECT_XPATH_TEXT(*it, "thr:count", "1");
}

#undef EXPECT_XPATH_TEXT

} // unnamed namespace
