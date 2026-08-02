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
#include "../include/book.h"
#include "../src/libopds_dumper.h"
#include "../src/tools/otherTools.h"

namespace
{

using namespace kiwix;

Book createBook(const std::string& id, const std::string& title)
{
  Book book;
  book.setId(id);
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

// Masks the document-level <updated> timestamp (always "now", via
// gen_date_str()). Per-book <updated>/<dc:issued> values are deterministic
// (derived from the book's own date) and are asserted verbatim.
std::string maskUpdatedTimestamp(std::string s)
{
  return replaceLines(s, R"(<updated>\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\dZ</updated>)",
                          "<updated>YYYY-MM-DDThh:mm:ssZ</updated>");
}

#define BOOK1_ENTRY \
    "  <entry>\n" \
    "    <id>urn:uuid:book1-id</id>\n" \
    "    <title>First Book</title>\n" \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
    "    <summary>Description of First Book</summary>\n" \
    "    <language>eng</language>\n" \
    "    <name>test_book1-id</name>\n" \
    "    <flavour></flavour>\n" \
    "    <category></category>\n" \
    "    <tags>tag1;tag2;_category:test</tags>\n" \
    "    <articleCount>42</articleCount>\n" \
    "    <mediaCount>7</mediaCount>\n" \
    "    \n" \
    "    <author>\n" \
    "      <name>First Book Creator</name>\n" \
    "    </author>\n" \
    "    <publisher>\n" \
    "      <name>First Book Publisher</name>\n" \
    "    </publisher>\n" \
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n" \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/book1-id.zim\" length=\"123456\" />\n" \
    "  </entry>\n"

#define BOOK2_ENTRY \
    "  <entry>\n" \
    "    <id>urn:uuid:book2-id</id>\n" \
    "    <title>Second Book</title>\n" \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
    "    <summary>Description of Second Book</summary>\n" \
    "    <language>eng</language>\n" \
    "    <name>test_book2-id</name>\n" \
    "    <flavour></flavour>\n" \
    "    <category></category>\n" \
    "    <tags>tag1;tag2;_category:test</tags>\n" \
    "    <articleCount>42</articleCount>\n" \
    "    <mediaCount>7</mediaCount>\n" \
    "    \n" \
    "    <author>\n" \
    "      <name>Second Book Creator</name>\n" \
    "    </author>\n" \
    "    <publisher>\n" \
    "      <name>Second Book Publisher</name>\n" \
    "    </publisher>\n" \
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n" \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/book2-id.zim\" length=\"123456\" />\n" \
    "  </entry>\n"

class LibOPDSDumperTest : public ::testing::Test {
 protected:
  void SetUp() override {
    lib = Library::create();
    lib->addBook(createBook("book1-id", "First Book"));
    lib->addBook(createBook("book2-id", "Second Book"));

    dumper.setLibrary(lib.get());
  }

  LibraryPtr lib;
  LibOPDSDumper dumper;
};

TEST_F(LibOPDSDumperTest, emptyBookIdListProducesNoEntries)
{
  std::ostringstream oss;
  dumper.dumpOPDSContent({}, oss);
  EXPECT_EQ(maskUpdatedTimestamp(oss.str()),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("/catalog") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "\n"
    "</feed>\n"
  );
}

TEST_F(LibOPDSDumperTest, dumpsRequestedBooksOnly)
{
  std::ostringstream oss;
  dumper.dumpOPDSContent({"book1-id"}, oss);
  EXPECT_EQ(maskUpdatedTimestamp(oss.str()),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("/catalog") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "\n"
    BOOK1_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibOPDSDumperTest, dumpsAllRequestedBooksInOrder)
{
  std::ostringstream oss;
  dumper.dumpOPDSContent({"book2-id", "book1-id"}, oss);
  EXPECT_EQ(maskUpdatedTimestamp(oss.str()),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("/catalog") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "\n"
    BOOK2_ENTRY
    BOOK1_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibOPDSDumperTest, unknownBookIdsAreSilentlyIgnored)
{
  std::ostringstream oss;
  dumper.dumpOPDSContent({"book1-id", "no-such-book", "book2-id"}, oss);
  EXPECT_EQ(maskUpdatedTimestamp(oss.str()),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("/catalog") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "\n"
    BOOK1_ENTRY
    BOOK2_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibOPDSDumperTest, defaultConstructedDumperHasNoLibrary)
{
  LibOPDSDumper freshDumper;
  std::ostringstream oss;
  freshDumper.dumpOPDSContent({"book1-id"}, oss);
  EXPECT_EQ(maskUpdatedTimestamp(oss.str()),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("/catalog") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "\n"
    "</feed>\n"
  );
}

TEST_F(LibOPDSDumperTest, setLibraryReplacesTheDumpedLibrary)
{
  auto otherLib = Library::create();
  otherLib->addBook(createBook("other-book-id", "Other Book"));

  dumper.setLibrary(otherLib.get());

  std::ostringstream oss;
  // "book1-id" belongs to the previously set library and is no longer known.
  dumper.dumpOPDSContent({"book1-id", "other-book-id"}, oss);
  EXPECT_EQ(maskUpdatedTimestamp(oss.str()),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("/catalog") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "\n"
    "  <entry>\n"
    "    <id>urn:uuid:other-book-id</id>\n"
    "    <title>Other Book</title>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <summary>Description of Other Book</summary>\n"
    "    <language>eng</language>\n"
    "    <name>test_other-book-id</name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags>tag1;tag2;_category:test</tags>\n"
    "    <articleCount>42</articleCount>\n"
    "    <mediaCount>7</mediaCount>\n"
    "    \n"
    "    <author>\n"
    "      <name>Other Book Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Other Book Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/other-book-id.zim\" length=\"123456\" />\n"
    "  </entry>\n"
    "</feed>\n"
  );
}

TEST_F(LibOPDSDumperTest, specialCharactersInBookMetadataAreEscaped)
{
  Book book = createBook("special-id", "Q&A <Tricky> \"Title\"");
  book.setDescription("Ideas & \"quotes\" <tags>");
  lib->addBook(book);

  std::ostringstream oss;
  dumper.dumpOPDSContent({"special-id"}, oss);
  EXPECT_EQ(maskUpdatedTimestamp(oss.str()),
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"
    "  <id>" + gen_uuid("/catalog") + "</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n"
    "\n"
    "  <entry>\n"
    "    <id>urn:uuid:special-id</id>\n"
    "    <title>Q&amp;A &lt;Tricky&gt; &quot;Title&quot;</title>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <summary>Ideas &amp; &quot;quotes&quot; &lt;tags&gt;</summary>\n"
    "    <language>eng</language>\n"
    "    <name>test_special-id</name>\n"
    "    <flavour></flavour>\n"
    "    <category></category>\n"
    "    <tags>tag1;tag2;_category:test</tags>\n"
    "    <articleCount>42</articleCount>\n"
    "    <mediaCount>7</mediaCount>\n"
    "    \n"
    "    <author>\n"
    "      <name>Q&amp;A &lt;Tricky&gt; &quot;Title&quot; Creator</name>\n"
    "    </author>\n"
    "    <publisher>\n"
    "      <name>Q&amp;A &lt;Tricky&gt; &quot;Title&quot; Publisher</name>\n"
    "    </publisher>\n"
    "    <dc:issued>2021-03-25T00:00:00Z</dc:issued>\n"
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"http://download.kiwix.org/zim/special-id.zim\" length=\"123456\" />\n"
    "  </entry>\n"
    "</feed>\n"
  );
}

#undef BOOK1_ENTRY
#undef BOOK2_ENTRY

} // unnamed namespace
