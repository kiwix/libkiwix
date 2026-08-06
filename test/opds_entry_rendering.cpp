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
    "    \n"
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
    "    \n"
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
    "    \n"
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

TEST(FullEntryOpdsTest, omitsContentLinkWhenContentAccessUrlIsEmpty)
{
  const Book book = createBook();
  // contentAccessUrl left at its default (empty).
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", /*contentAccessUrl=*/"", "book-id"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    \n"
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

TEST(FullEntryOpdsTest, omitsSelfLinkWhenSelfPathIsEmpty)
{
  const Book book = createBook();
  // selfPath left at its default ("") - mirrors how OPDSDumper calls this
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

TEST(FullEntryOpdsTest, rendersSelfLinkWhenSelfPathIsSet)
{
  const Book book = createBook();
  EXPECT_EQ(fullEntryOpds(book, "http://root.location", "", "book-id",
                          /*selfPath=*/"/local/path/book.zim"),
    "  <entry>\n"
    CORE_ENTRY_BODY
    "    <author>\n"
    "      <name>Some Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Some Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    <link rel=\"self\" href=\"/local/path/book.zim\" type=\"application/x-zim\"/>\n"
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
    "    \n"
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
