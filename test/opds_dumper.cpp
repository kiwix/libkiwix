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

#include <regex>
#include <sstream>

#include "../include/library.h"
#include "../include/manager.h"
#include "../include/name_mapper.h"
#include "../include/book.h"
#include "../include/tools.h"
#include "../src/opds_dumper.h"
#include "../src/tools/otherTools.h"

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

// Returns a copy of 'text' where every line that fully matches 'pattern'
// (preceded by optional whitespace) is replaced with 'replacement',
// preserving the leading whitespace. Copied from test/library_server.cpp,
// which uses the same technique for the same reason (masking timestamps
// generated from the current wall-clock time).
std::string replaceLines(const std::string& text,
                         const std::string& pattern,
                         const std::string& replacement)
{
  std::regex regex("^ *" + pattern + "$");
  std::ostringstream oss;
  std::istringstream iss(text);
  std::string line;
  while ( std::getline(iss, line) ) {
    if ( std::regex_match(line, regex) ) {
      for ( size_t i = 0; i < line.size() && line[i] == ' '; ++i )
        oss << ' ';
      oss << replacement << "\n";
    } else {
      oss << line << "\n";
    }
  }
  return oss.str();
}

// Masks every <updated>...</updated> timestamp - both the feed-level one
// (always "now", via gen_date_str()) and the per-entry/per-category/
// per-language ones used by categoriesOPDSFeed()/languagesOPDSFeed() (also
// "now" - see LibraryDumper::getCategoryData()/getLanguageData()). Per-book
// entries elsewhere carry a real, deterministic book date, but this matches
// them too, same as library_server.cpp's CATALOG_ENTRY does for the live
// HTTP catalog - the trade-off of a slightly less precise assertion in
// exchange for a much simpler, more robust one.
std::string maskUpdatedTimestamps(std::string s)
{
  return replaceLines(s, R"(<updated>\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\dZ</updated>)",
                          "<updated>YYYY-MM-DDThh:mm:ssZ</updated>");
}

// Compares 'actual' (an OPDS feed/entry string, possibly containing
// wall-clock-generated <updated> timestamps) against 'expected' after
// masking those timestamps out of 'actual', so tests don't have to spell
// out maskUpdatedTimestamps(...) at every assertion.
#define EXPECT_OPDS(actual, expected) \
    EXPECT_EQ(maskUpdatedTimestamps(actual), (expected))

#define BOOK1_ENTRY \
    "  <entry>\n" \
    "    <id>urn:uuid:book1-id</id>\n" \
    "    <title>First Book</title>\n" \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
    "    <summary>Description of First Book</summary>\n" \
    "    <language>eng</language>\n" \
    "    <name>test_book1-id</name>\n" \
    "    <flavour></flavour>\n" \
    "    <category>wikipedia</category>\n" \
    "    <tags>tag1;tag2;_category:wikipedia</tags>\n" \
    "    <articleCount>42</articleCount>\n" \
    "    <mediaCount>7</mediaCount>\n" \
    "    <link type=\"text/html\" href=\"http://root.location/content/book1-id\" />\n" \
    "    <author>\n" \
    "      <name>First Book Creator</name>\n" \
    "    </author>\n" \
    "    <publisher>\n" \
    "      <name>First Book Publisher</name>\n" \
    "    </publisher>\n" \
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n" \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/book1-id.zim\" length=\"122880\" />\n" \
    "  </entry>\n"

#define BOOK2_ENTRY \
    "  <entry>\n" \
    "    <id>urn:uuid:book2-id</id>\n" \
    "    <title>Second Book</title>\n" \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
    "    <summary>Description of Second Book</summary>\n" \
    "    <language>fra</language>\n" \
    "    <name>test_book2-id</name>\n" \
    "    <flavour></flavour>\n" \
    "    <category>wiktionary</category>\n" \
    "    <tags>tag3</tags>\n" \
    "    <articleCount>10</articleCount>\n" \
    "    <mediaCount>1</mediaCount>\n" \
    "    <link type=\"text/html\" href=\"http://root.location/content/book2-id\" />\n" \
    "    <author>\n" \
    "      <name>Second Book Creator</name>\n" \
    "    </author>\n" \
    "    <publisher>\n" \
    "      <name>Second Book Publisher</name>\n" \
    "    </publisher>\n" \
    "    <dc:issued>2021-03-26T00:00:00Z</dc:issued>\n" \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/book2-id.zim\" length=\"46080\" />\n" \
    "  </entry>\n"

#define BOOK1_PARTIAL_ENTRY \
    "  <entry>\n" \
    "    <id>urn:uuid:book1-id</id>\n" \
    "    <title>First Book</title>\n" \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
    "    <link rel=\"alternate\"\n" \
    "          href=\"http://root.location/catalog/v2/entry/book1-id\"\n" \
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n" \
    "  </entry>\n"

#define BOOK2_PARTIAL_ENTRY \
    "  <entry>\n" \
    "    <id>urn:uuid:book2-id</id>\n" \
    "    <title>Second Book</title>\n" \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
    "    <link rel=\"alternate\"\n" \
    "          href=\"http://root.location/catalog/v2/entry/book2-id\"\n" \
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n" \
    "  </entry>\n"

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

TEST_F(OPDSDumperTest, dumpOPDSFeedWithoutFilteringInfo)
{
  EXPECT_OPDS(dumper->dumpOPDSFeed({"book1-id", "book2-id"}, ""),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("test-library-id/catalog/search?") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "  <link rel=\"search\" type=\"application/opensearchdescription+xml\" href=\"http://root.location/catalog/searchdescription.xml\" />\n"
    BOOK1_ENTRY
    BOOK2_ENTRY
    "</feed>\n"
  );
}

TEST_F(OPDSDumperTest, dumpOPDSFeedWithFilteringInfo)
{
  EXPECT_OPDS(dumper->dumpOPDSFeed({"book1-id"}, "lang=eng"),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("test-library-id/catalog/search?lang=eng") + "</id>\n"
    "  <title>Filtered zims (lang=eng)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "  <link rel=\"search\" type=\"application/opensearchdescription+xml\" href=\"http://root.location/catalog/searchdescription.xml\" />\n"
    BOOK1_ENTRY
    "</feed>\n"
  );
}

TEST_F(OPDSDumperTest, dumpOPDSFeedV2FullEntries)
{
  EXPECT_OPDS(dumper->dumpOPDSFeedV2({"book1-id", "book2-id"}, "", /*partial=*/false),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\"\n"
    "      xmlns:opensearch=\"http://a9.com/-/spec/opensearch/1.1/\">\n"
    "  <id>" + gen_uuid("test-library-id/entries?") + "</id>\n"
    "\n"
    "  <link rel=\"self\"\n"
    "        href=\"http://root.location/catalog/v2/entries\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n"
    "  <link rel=\"start\"\n"
    "        href=\"http://root.location/catalog/v2/root.xml\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "  <link rel=\"up\"\n"
    "        href=\"http://root.location/catalog/v2/root.xml\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "\n"
    "  <title>All Entries</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    BOOK1_ENTRY
    BOOK2_ENTRY
    "</feed>\n"
  );
}

TEST_F(OPDSDumperTest, dumpOPDSFeedV2PartialEntries)
{
  EXPECT_OPDS(dumper->dumpOPDSFeedV2({"book1-id", "book2-id"}, "", /*partial=*/true),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\"\n"
    "      xmlns:opensearch=\"http://a9.com/-/spec/opensearch/1.1/\">\n"
    "  <id>" + gen_uuid("test-library-id/partial_entries?") + "</id>\n"
    "\n"
    "  <link rel=\"self\"\n"
    "        href=\"http://root.location/catalog/v2/partial_entries\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n"
    "  <link rel=\"start\"\n"
    "        href=\"http://root.location/catalog/v2/root.xml\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "  <link rel=\"up\"\n"
    "        href=\"http://root.location/catalog/v2/root.xml\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "\n"
    "  <title>All Entries</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    BOOK1_PARTIAL_ENTRY
    BOOK2_PARTIAL_ENTRY
    "</feed>\n"
  );
}

TEST_F(OPDSDumperTest, dumpOPDSCompleteEntryProducesStandaloneEntryDocument)
{
  EXPECT_OPDS(dumper->dumpOPDSCompleteEntry("book2-id"),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    BOOK2_ENTRY
  );
}

TEST_F(OPDSDumperTest, dumpOPDSCompleteEntryThrowsForUnknownBookId)
{
  EXPECT_THROW(dumper->dumpOPDSCompleteEntry("no-such-book"), std::out_of_range);
}

TEST_F(OPDSDumperTest, categoriesOPDSFeedListsDistinctSortedCategories)
{
  // Categories are listed in sorted order: "wikipedia" (from the
  // "_category:" tag on book1) before "wiktionary" (from book2's explicit
  // "category" attribute).
  EXPECT_OPDS(dumper->categoriesOPDSFeed(),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\">\n"
    "  <id>" + gen_uuid("test-library-id/categories") + "</id>\n"
    "  <link rel=\"self\"\n"
    "        href=\"http://root.location/catalog/v2/categories\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "  <link rel=\"start\"\n"
    "        href=\"http://root.location/catalog/v2/root.xml\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "  <title>List of categories</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <entry>\n"
    "    <title>wikipedia</title>\n"
    "    <link rel=\"subsection\"\n"
    "          href=\"http://root.location/catalog/v2/entries?category=wikipedia\"\n"
    "          type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <id>" + gen_uuid("test-library-id/categories/wikipedia") + "</id>\n"
    "    <content type=\"text\">All entries with category of 'wikipedia'.</content>\n"
    "  </entry>\n"
    "  <entry>\n"
    "    <title>wiktionary</title>\n"
    "    <link rel=\"subsection\"\n"
    "          href=\"http://root.location/catalog/v2/entries?category=wiktionary\"\n"
    "          type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <id>" + gen_uuid("test-library-id/categories/wiktionary") + "</id>\n"
    "    <content type=\"text\">All entries with category of 'wiktionary'.</content>\n"
    "  </entry>\n"
    "</feed>\n"
  );
}

TEST_F(OPDSDumperTest, languagesOPDSFeedListsLanguagesWithCounts)
{
  // Languages are listed in sorted order of their ISO code: "eng" before "fra".
  EXPECT_OPDS(dumper->languagesOPDSFeed(),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\"\n"
    "      xmlns:thr=\"http://purl.org/syndication/thread/1.0\">\n"
    "  <id>" + gen_uuid("test-library-id/languages") + "</id>\n"
    "  <link rel=\"self\"\n"
    "        href=\"http://root.location/catalog/v2/languages\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "  <link rel=\"start\"\n"
    "        href=\"http://root.location/catalog/v2/root.xml\"\n"
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n"
    "  <title>List of languages</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <entry>\n"
    "    <title>" + getLanguageSelfName("eng") + "</title>\n"
    "    <dc:language>eng</dc:language>\n"
    "    <thr:count>1</thr:count>\n"
    "    <link rel=\"subsection\"\n"
    "          href=\"http://root.location/catalog/v2/entries?lang=eng\"\n"
    "          type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <id>" + gen_uuid("test-library-id/languages/eng") + "</id>\n"
    "  </entry>\n"
    "  <entry>\n"
    "    <title>" + getLanguageSelfName("fra") + "</title>\n"
    "    <dc:language>fra</dc:language>\n"
    "    <thr:count>1</thr:count>\n"
    "    <link rel=\"subsection\"\n"
    "          href=\"http://root.location/catalog/v2/entries?lang=fra\"\n"
    "          type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <id>" + gen_uuid("test-library-id/languages/fra") + "</id>\n"
    "  </entry>\n"
    "</feed>\n"
  );
}

#undef EXPECT_OPDS
#undef BOOK1_ENTRY
#undef BOOK2_ENTRY
#undef BOOK2_PARTIAL_ENTRY
#undef BOOK1_PARTIAL_ENTRY

} // unnamed namespace
