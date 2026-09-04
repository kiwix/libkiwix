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
#include "testing_tools.h"

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

#define CORE_ENTRY_BODY \
    "    <id>urn:uuid:book-id</id>\n" \
    "    <title>Some Title</title>\n" \
    "    <updated>2021-03-25T00:00:00Z</updated>\n" \
    "    <summary>Some Description</summary>\n" \
    "    <language>eng</language>\n" \
    "    <name>some_name</name>\n" \
    "    <flavour>nopic</flavour>\n" \
    /* Book::getCategory() only reflects a "_category:" tag/attribute      */ \
    /* resolved during XML/OPDS parsing - it is not derived on the fly    */ \
    /* from setTags(), so a manually-constructed Book always reports an   */ \
    /* empty category here.                                              */ \
    "    <category></category>\n" \
    "    <tags>tag1;tag2;_category:wikipedia</tags>\n" \
    "    <articleCount>42</articleCount>\n" \
    "    <mediaCount>7</mediaCount>\n"

TEST(FullEntryOpdsTest, rendersCoreBookFields)
{
  const Book book = createBook();
  // No url and no contentAccessUrl were set, so neither the acquisition nor
  // the content <link> is rendered (see rendersAcquisitionLinkWhenUrlIsSet /
  // rendersUrlEncodedContentLinkWhenContentAccessUrlIsSet below for when
  // they are).
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", /*contentAccessUrl=*/"", /*contentId=*/"book-id"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, omitsAcquisitionLinkWhenUrlIsEmpty)
{
  const Book book = createBook();
  // url left at its default (empty).
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-id"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersAcquisitionLinkWhenUrlIsSet)
{
  Book book = createBook();
  book.setUrl("http://download.kiwix.org/zim/book.zim");
  book.setSize(123456);

  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-id"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/book.zim\" length=\"123456\" />\n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersOneLinkPerAcquisitionLinkMimeType)
{
  Book book = createBook();
  book.setUrl("http://download.kiwix.org/zim/book.zim");
  book.setUrl(Book::ACQUISITION_MIMETYPE_ZIM_METALINK, "http://download.kiwix.org/zim/book.zim.meta4");
  book.setUrl("application/x-bittorrent", "http://download.kiwix.org/zim/book.zim.torrent");
  book.setSize(123456);

  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-id"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/book.zim\" length=\"123456\" />\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/metalink4+xml\" href=\"http://download.kiwix.org/zim/book.zim.meta4\" length=\"123456\" />\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-bittorrent\" href=\"http://download.kiwix.org/zim/book.zim.torrent\" length=\"123456\" />\n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, omitsContentLinkWhenContentAccessUrlIsEmpty)
{
  const Book book = createBook();
  // contentAccessUrl left at its default (empty).
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", /*contentAccessUrl=*/"", "book-id"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersUrlEncodedContentLinkWhenContentAccessUrlIsSet)
{
  const Book book = createBook();
  // '/' is one of urlEncode()'s "harmless" characters (paths may legitimately
  // contain it) and passes through untouched, while ' ' and '#' do not.
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "http://root.location/content", "a/b c#d"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <link type=\"text/html\" href=\"http://root.location/content/a/b%20c%23d\" />\n"
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
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

  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-with-icon"),
    "  <entry>\n"
    "    <id>urn:uuid:book-with-icon</id>\n"
    "    <title>Book With Icon</title>\n"
    "    <updated>T00:00:00Z</updated>\n"
    "    <summary></summary>\n"
    "    <language></language>\n"
    "    <name></name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags></tags>\n"
    "    <articleCount>0</articleCount>\n"
    "    <mediaCount>0</mediaCount>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"http://root.location/catalog/v2/illustration/book-with-icon/?size=48\"\n"
    "          type=\"image/png;width=48;height=48;scale=1\"/>\n"
    "    <author>\n"
    "      <name></name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name></name>\n"
    "    </publisher>\n"
    "    <dc:issued>T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersBase64ThumbnailForEmbeddedOnlyIllustrationInFileDump)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleXML[] = R"(
<library version="1.0">
  <book
        id="book-with-embedded-icon"
        path="/local/path/book.zim"
        title="Book With Embedded Icon"
        favicon="AAAA"
        faviconMimeType="image/png"
      ></book>
</library>
)";
  manager.readXml(sampleXML, /*readOnly=*/false, "", /*trustLibrary=*/true);
  const Book& book = lib->getBookById("book-with-embedded-icon");

  // isLiveCatalog=false (offline file dump): an illustration with no
  // external url is only available as embedded data, and there is no server
  // behind /catalog/v2/illustration there to serve it from, so it is
  // embedded directly as a data: URI in the thumbnail link's href instead
  // (OPDS 1.2 5.2.2).
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-with-embedded-icon",
                          /*localPath=*/"", /*isLiveCatalog=*/false),
    "  <entry>\n"
    "    <id>urn:uuid:book-with-embedded-icon</id>\n"
    "    <title>Book With Embedded Icon</title>\n"
    "    <updated>T00:00:00Z</updated>\n"
    "    <summary></summary>\n"
    "    <language></language>\n"
    "    <name></name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags></tags>\n"
    "    <articleCount>0</articleCount>\n"
    "    <mediaCount>0</mediaCount>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"data:image/png;base64,AAAA\"\n"
    "          type=\"image/png;width=48;height=48;scale=1\"/>\n"
    "    <author>\n"
    "      <name></name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name></name>\n"
    "    </publisher>\n"
    "    <dc:issued>T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersIllustrationUrlDirectlyInFileDump)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleXML[] = R"(
<library version="1.0">
  <book
        id="book-with-icon-url"
        path="/local/path/book.zim"
        title="Book With Icon Url"
        favicon="https://example.com/favicon/zara.png"
        faviconMimeType="image/png"
        faviconUrl="/favicon.png"
      ></book>
</library>
)";
  manager.readXml(sampleXML, /*readOnly=*/false, "", /*trustLibrary=*/true);
  const Book& book = lib->getBookById("book-with-icon-url");

  // isLiveCatalog=false (offline file dump): with an external url available,
  // the thumbnail link points straight at that url instead of the live
  // /catalog/v2/illustration endpoint, which has no server behind it there.
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-with-icon-url",
                          /*localPath=*/"", /*isLiveCatalog=*/false),
    "  <entry>\n"
    "    <id>urn:uuid:book-with-icon-url</id>\n"
    "    <title>Book With Icon Url</title>\n"
    "    <updated>T00:00:00Z</updated>\n"
    "    <summary></summary>\n"
    "    <language></language>\n"
    "    <name></name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags></tags>\n"
    "    <articleCount>0</articleCount>\n"
    "    <mediaCount>0</mediaCount>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"/favicon.png\"\n"
    "          type=\"image/png;width=48;height=48;scale=1\"/>\n"
    "    <author>\n"
    "      <name></name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name></name>\n"
    "    </publisher>\n"
    "    <dc:issued>T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, doesNotDownloadIllustrationDataForLinkBasedThumbnail)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleOpds[] = R"(
<feed>
  <entry>
    <id>book-with-remote-icon</id>
    <title>Book With Remote Icon</title>
    <link rel="http://opds-spec.org/image/thumbnail"
          type="image/png"
          href="http://127.0.0.1:1/unreachable-favicon.png" />
  </entry>
</feed>
)";
  ASSERT_TRUE(manager.readOpds(sampleOpds, "http://root.location"));
  const Book& book = lib->getBookById("book-with-remote-icon");
  ASSERT_EQ(book.getIllustrations().size(), 1U);

  kiwix::testing::CapturedStderr stderror;
  fullEntryOpds(book, "http://root.location", "", "book-with-remote-icon");

  // XXX This is a fragile way of testing the sought behaviour relying on the fact
  // that Book::Illustration::getData() prints to stderr if it tries to
  // download the thumbnail data. A more robust test would start an HTTP
  // server and check that no request has been made to it.
  EXPECT_EQ(std::string(stderror), "");
  EXPECT_TRUE(book.getIllustrations().at(0)->getData().empty());
}

TEST(FullEntryOpdsTest, roundTripsBase64ThumbnailDataThroughOPDSReadback)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleXML[] = R"(
<library version="1.0">
  <book
        id="book-with-roundtrip-icon"
        path="/local/path/book.zim"
        title="Book With Roundtrip Icon"
        favicon="SGVsbG8sIFdvcmxkIQ=="
        faviconMimeType="image/png"
      ></book>
</library>
)";
  manager.readXml(sampleXML, /*readOnly=*/false, "", /*trustLibrary=*/true);
  const Book& book = lib->getBookById("book-with-roundtrip-icon");
  ASSERT_EQ(book.getIllustrations().at(0)->getData(), "Hello, World!");

  const std::string rendered = fullEntryOpds(book, "http://root.location", "", "book-with-roundtrip-icon",
                                              /*localPath=*/"", /*isLiveCatalog=*/false);

  // Guards against the class of bug where the template puts whitespace (e.g.
  // a newline+indentation) around the base64 payload: base64_decode() stops
  // at the first non-base64 character rather than skipping it, so such
  // whitespace silently turns the decoded data into an empty string. A plain
  // golden-string comparison wouldn't catch this if the expected string were
  // updated to match a broken rendering, so decode the actual rendered
  // output through the real OPDS reader (Manager::readOpds()) instead.
  auto roundTripLib = Library::create();
  Manager roundTripManager(roundTripLib);
  ASSERT_TRUE(roundTripManager.readOpds("<feed>" + rendered + "</feed>", "http://root.location"));
  const Book& roundTrippedBook = roundTripLib->getBookById("book-with-roundtrip-icon");

  ASSERT_EQ(roundTrippedBook.getIllustrations().size(), 1U);
  EXPECT_EQ(roundTrippedBook.getIllustrations().at(0)->getData(), "Hello, World!");
}

TEST(FullEntryOpdsTest, rendersMultipleBase64Thumbnails)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleOpds[] = R"(
<feed>
  <entry>
    <id>multi-icon-book</id>
    <title>Book With Multiple Icons</title>
    <link rel="http://opds-spec.org/image/thumbnail"
          type="image/png;width=48;height=48;scale=1"
          href="data:image/png;base64,Zmlyc3QtdGh1bWJuYWls" />
    <link rel="http://opds-spec.org/image/thumbnail"
          type="image/jpeg;width=96;height=96;scale=1"
          href="data:image/jpeg;base64,c2Vjb25kLXRodW1ibmFpbA==" />
  </entry>
</feed>
)";
  ASSERT_TRUE(manager.readOpds(sampleOpds, "http://root.location"));
  const Book& book = lib->getBookById("multi-icon-book");

  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "multi-icon-book",
                           /*localPath=*/"", /*isLiveCatalog=*/false),
    "  <entry>\n"
    "    <id>urn:uuid:multi-icon-book</id>\n"
    "    <title>Book With Multiple Icons</title>\n"
    "    <updated>T00:00:00Z</updated>\n"
    "    <summary></summary>\n"
    "    <language></language>\n"
    "    <name></name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags></tags>\n"
    "    <articleCount>0</articleCount>\n"
    "    <mediaCount>0</mediaCount>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"data:image/png;base64,Zmlyc3QtdGh1bWJuYWls\"\n"
    "          type=\"image/png;width=48;height=48;scale=1\"/>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"data:image/jpeg;base64,c2Vjb25kLXRodW1ibmFpbA==\"\n"
    "          type=\"image/jpeg;width=96;height=96;scale=1\"/>\n"
    "    <author>\n"
    "      <name></name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name></name>\n"
    "    </publisher>\n"
    "    <dc:issued>T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersMixedLinkAndBase64ThumbnailsInFileDump)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleOpds[] = R"(
<feed>
  <entry>
    <id>mixed-icon-book</id>
    <title>Book With Mixed Icons</title>
    <link rel="http://opds-spec.org/image/thumbnail"
          type="image/png"
          href="https://example.com/favicon.png" />
    <link rel="http://opds-spec.org/image/thumbnail"
          type="image/jpeg;width=48;height=48;scale=1"
          href="data:image/jpeg;base64,Zmlyc3QtdGh1bWJuYWls" />
  </entry>
</feed>
)";
  ASSERT_TRUE(manager.readOpds(sampleOpds, "http://root.location"));
  const Book& book = lib->getBookById("mixed-icon-book");

  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "mixed-icon-book",
                           /*localPath=*/"", /*isLiveCatalog=*/false),
    "  <entry>\n"
    "    <id>urn:uuid:mixed-icon-book</id>\n"
    "    <title>Book With Mixed Icons</title>\n"
    "    <updated>T00:00:00Z</updated>\n"
    "    <summary></summary>\n"
    "    <language></language>\n"
    "    <name></name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags></tags>\n"
    "    <articleCount>0</articleCount>\n"
    "    <mediaCount>0</mediaCount>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"https://example.com/favicon.png\"\n"
    "          type=\"image/png;width=48;height=48;scale=1\"/>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"data:image/jpeg;base64,Zmlyc3QtdGh1bWJuYWls\"\n"
    "          type=\"image/jpeg;width=48;height=48;scale=1\"/>\n"
    "    <author>\n"
    "      <name></name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name></name>\n"
    "    </publisher>\n"
    "    <dc:issued>T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, omitsDataUriThumbnailForEmbeddedOnlyIllustrationInLiveCatalog)
{
  auto lib = Library::create();
  Manager manager(lib);
  const char sampleXML[] = R"(
<library version="1.0">
  <book
        id="book-with-embedded-icon"
        path="/local/path/book.zim"
        title="Book With Embedded Icon"
        favicon="AAAA"
        faviconMimeType="image/png"
      ></book>
</library>
)";
  manager.readXml(sampleXML, /*readOnly=*/false, "", /*trustLibrary=*/true);
  const Book& book = lib->getBookById("book-with-embedded-icon");

  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-with-embedded-icon"),
    "  <entry>\n"
    "    <id>urn:uuid:book-with-embedded-icon</id>\n"
    "    <title>Book With Embedded Icon</title>\n"
    "    <updated>T00:00:00Z</updated>\n"
    "    <summary></summary>\n"
    "    <language></language>\n"
    "    <name></name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags></tags>\n"
    "    <articleCount>0</articleCount>\n"
    "    <mediaCount>0</mediaCount>\n"
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"http://root.location/catalog/v2/illustration/book-with-embedded-icon/?size=48\"\n"
    "          type=\"image/png;width=48;height=48;scale=1\"/>\n"
    "    <author>\n"
    "      <name></name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name></name>\n"
    "    </publisher>\n"
    "    <dc:issued>T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, omitsLocalPathAcquisitionLinkWhenEmpty)
{
  const Book book = createBook();
  // localPath left at its default ("") - mirrors how OPDSDumper calls this
  // function for the live HTTP catalog, which must never leak a local path.
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-id"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersLocalPathAcquisitionLinkWhenSet)
{
  Book book = createBook();
  book.setSize(123456);
  EXPECT_EQ(fullEntryOpds(book, /*rootLocation=*/"", "", "book-id",
                          /*localPath=*/"/local/path/book.zim"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" href=\"/local/path/book.zim\" type=\"application/x-zim\" length=\"123456\"/>\n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, rendersBothAcquisitionLinksWhenUrlAndLocalPathAreSet)
{
  Book book = createBook();
  book.setUrl("http://download.kiwix.org/zim/book.zim");
  book.setSize(123456);

  EXPECT_EQ(fullEntryOpds(book, /*rootLocation=*/"", "", "book-id",
                          /*localPath=*/"/local/path/book.zim"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/book.zim\" length=\"123456\" />\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" href=\"/local/path/book.zim\" type=\"application/x-zim\" length=\"123456\"/>\n"
    "  </entry>\n"
  );
}

TEST(FullEntryOpdsTest, specialCharactersInBookMetadataAreEscaped)
{
  Book book = createBook();
  book.setTitle("Q&A <Tricky> \"Title\"");
  book.setDescription("Ideas & \"quotes\" <tags>");

  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-id"),
    "  <entry>\n"
    "    <id>urn:uuid:book-id</id>\n"
    "    <title>Q&amp;A &lt;Tricky&gt; &quot;Title&quot;</title>\n"
    "    <updated>2021-03-25T00:00:00Z</updated>\n"
    "    <summary>Ideas &amp; &quot;quotes&quot; &lt;tags&gt;</summary>\n"
    "    <language>eng</language>\n"
    "    <name>some_name</name>\n"
    "    <flavour>nopic</flavour>\n"
    "    <category></category>\n"
    "    <tags>tag1;tag2;_category:wikipedia</tags>\n"
    "    <articleCount>42</articleCount>\n"
    "    <mediaCount>7</mediaCount>\n"
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    \n"
    "  </entry>\n"
  );
}

#undef CORE_ENTRY_BODY

} // unnamed namespace
