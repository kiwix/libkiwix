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

#include "../src/libxml_dumper.h"

#include "../include/book.h"
#include "../include/bookmark.h"
#include "../include/library.h"
#include "../include/tools.h"
#include "gtest/gtest.h"

namespace
{

using namespace kiwix;

Book createBook(const std::string& id, const std::string& title)
{
  Book book;
  book.setId(id);
  // A relative path is resolved by Book::setPath() against the current
  // directory; the test dumper's baseDir is set to that same directory
  // below, so the dumped "path" attribute is portably just "<id>.zim" on
  // every platform (a hardcoded absolute path like "/local/path/..." is
  // POSIX-only - Windows treats a leading '/' without a drive letter as
  // relative, which silently prefixes it with the build's cwd instead).
  book.setPath(id + ".zim");
  book.setTitle(title);
  book.setDescription("Description of " + title);
  book.setLanguage("eng,spa");
  book.setCreator(title + " Creator");
  book.setPublisher(title + " Publisher");
  book.setDate("2021-03-25");
  book.setUrl("http://download.kiwix.org/zim/" + id + ".zim");
  book.setName("test_" + id);
  book.setFlavour("nopic");
  book.setTags("tag1;tag2");
  book.setArticleCount(42);
  book.setMediaCount(7);
  book.setSize(122880);  // 120 KiB - stored on disk as KiB (>>10)
  book.setDownloadId("download-" + id);
  return book;
}

class LibXMLDumperTest : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    lib = Library::create();
    lib->addBook(createBook("book1-id", "First Book"));
    lib->addBook(createBook("book2-id", "Second Book"));

    dumper.setLibrary(lib.get());
    dumper.setBaseDir(getCurrentDirectory());
  }

  LibraryPtr lib;
  LibXMLDumper dumper;
};

TEST_F(LibXMLDumperTest, emptyBookIdListProducesEmptyLibraryElement)
{
  EXPECT_EQ(dumper.dumpLibXMLContent({}),
    "<library version=\"20110515\" />\n"
  );
}

TEST_F(LibXMLDumperTest, dumpsRequestedBookWithAllAttributes)
{
  EXPECT_EQ(dumper.dumpLibXMLContent({"book1-id"}),
    "<library version=\"20110515\">\n"
    // The book's path is stored relative to the dumper's base dir, and
    // size on disk in KiB (bytes >> 10).
    "  <book id=\"book1-id\" path=\"book1-id.zim\" title=\"First Book\""
    " description=\"Description of First Book\" language=\"eng,spa\""
    " creator=\"First Book Creator\" publisher=\"First Book Publisher\""
    " name=\"test_book1-id\" flavour=\"nopic\" tags=\"tag1;tag2\""
    " date=\"2021-03-25\" url=\"http://download.kiwix.org/zim/book1-id.zim\""
    " articleCount=\"42\" mediaCount=\"7\" size=\"120\""
    " downloadId=\"download-book1-id\" />\n"
    "</library>\n"
  );
}

TEST_F(LibXMLDumperTest, dumpsOnlyRequestedBooksInOrder)
{
  EXPECT_EQ(dumper.dumpLibXMLContent({"book2-id", "book1-id"}),
    "<library version=\"20110515\">\n"
    "  <book id=\"book2-id\" path=\"book2-id.zim\" title=\"Second Book\""
    " description=\"Description of Second Book\" language=\"eng,spa\""
    " creator=\"Second Book Creator\" publisher=\"Second Book Publisher\""
    " name=\"test_book2-id\" flavour=\"nopic\" tags=\"tag1;tag2\""
    " date=\"2021-03-25\" url=\"http://download.kiwix.org/zim/book2-id.zim\""
    " articleCount=\"42\" mediaCount=\"7\" size=\"120\""
    " downloadId=\"download-book2-id\" />\n"
    "  <book id=\"book1-id\" path=\"book1-id.zim\" title=\"First Book\""
    " description=\"Description of First Book\" language=\"eng,spa\""
    " creator=\"First Book Creator\" publisher=\"First Book Publisher\""
    " name=\"test_book1-id\" flavour=\"nopic\" tags=\"tag1;tag2\""
    " date=\"2021-03-25\" url=\"http://download.kiwix.org/zim/book1-id.zim\""
    " articleCount=\"42\" mediaCount=\"7\" size=\"120\""
    " downloadId=\"download-book1-id\" />\n"
    "</library>\n"
  );
}

TEST_F(LibXMLDumperTest, omitsEmptyOrZeroOptionalAttributes)
{
  Book book = createBook("bare-book-id", "Bare Book");
  book.setUrl("");
  book.setDownloadId("");
  book.setArticleCount(0);
  book.setMediaCount(0);
  book.setSize(0);
  lib->addBook(book);

  EXPECT_EQ(dumper.dumpLibXMLContent({"bare-book-id"}),
    "<library version=\"20110515\">\n"
    "  <book id=\"bare-book-id\" path=\"bare-book-id.zim\" title=\"Bare Book\""
    " description=\"Description of Bare Book\" language=\"eng,spa\""
    " creator=\"Bare Book Creator\" publisher=\"Bare Book Publisher\""
    " name=\"test_bare-book-id\" flavour=\"nopic\" tags=\"tag1;tag2\""
    " date=\"2021-03-25\" />\n"
    "</library>\n"
  );
}

TEST_F(LibXMLDumperTest, skipsReadOnlyBooks)
{
  Book readOnlyBook = createBook("readonly-book-id", "Read Only Book");
  readOnlyBook.setReadOnly(true);
  lib->addBook(readOnlyBook);

  // The read-only book is entirely omitted from the dump, not just stripped
  // of its attributes.
  EXPECT_EQ(dumper.dumpLibXMLContent({"book1-id", "readonly-book-id"}),
    "<library version=\"20110515\">\n"
    "  <book id=\"book1-id\" path=\"book1-id.zim\" title=\"First Book\""
    " description=\"Description of First Book\" language=\"eng,spa\""
    " creator=\"First Book Creator\" publisher=\"First Book Publisher\""
    " name=\"test_book1-id\" flavour=\"nopic\" tags=\"tag1;tag2\""
    " date=\"2021-03-25\" url=\"http://download.kiwix.org/zim/book1-id.zim\""
    " articleCount=\"42\" mediaCount=\"7\" size=\"120\""
    " downloadId=\"download-book1-id\" />\n"
    "</library>\n"
  );
}

TEST_F(LibXMLDumperTest, aliasBookWithOrigIdOnlyEmitsIdAndOrigId)
{
  Book aliasBook = createBook("alias-book-id", "Alias Book");
  aliasBook.setOrigId("book1-id");
  lib->addBook(aliasBook);

  // Metadata that's only meaningful for the "real" entry is skipped for a
  // flavour alias - only path (above the origId/title branch) and
  // date/url/counts/size/downloadId (below it) are still emitted.
  EXPECT_EQ(dumper.dumpLibXMLContent({"alias-book-id"}),
    "<library version=\"20110515\">\n"
    "  <book id=\"alias-book-id\" path=\"alias-book-id.zim\" origId=\"book1-id\""
    " date=\"2021-03-25\" url=\"http://download.kiwix.org/zim/alias-book-id.zim\""
    " articleCount=\"42\" mediaCount=\"7\" size=\"120\""
    " downloadId=\"download-alias-book-id\" />\n"
    "</library>\n"
  );
}

TEST_F(LibXMLDumperTest, dumpLibXMLContentThrowsForUnknownBookId)
{
  EXPECT_THROW(dumper.dumpLibXMLContent({"no-such-book"}), std::out_of_range);
}

TEST_F(LibXMLDumperTest, defaultConstructedDumperHasNoLibrary)
{
  LibXMLDumper freshDumper;

  EXPECT_EQ(freshDumper.dumpLibXMLContent({"book1-id"}),
    "<library version=\"20110515\" />\n"
  );
  EXPECT_EQ(freshDumper.dumpLibXMLBookmark(),
    "<bookmarks />\n"
  );
}

TEST_F(LibXMLDumperTest, setLibraryReplacesTheDumpedLibrary)
{
  auto otherLib = Library::create();
  otherLib->addBook(createBook("other-book-id", "Other Book"));

  dumper.setLibrary(otherLib.get());

  EXPECT_EQ(dumper.dumpLibXMLContent({"other-book-id"}),
    "<library version=\"20110515\">\n"
    "  <book id=\"other-book-id\" path=\"other-book-id.zim\" title=\"Other Book\""
    " description=\"Description of Other Book\" language=\"eng,spa\""
    " creator=\"Other Book Creator\" publisher=\"Other Book Publisher\""
    " name=\"test_other-book-id\" flavour=\"nopic\" tags=\"tag1;tag2\""
    " date=\"2021-03-25\" url=\"http://download.kiwix.org/zim/other-book-id.zim\""
    " articleCount=\"42\" mediaCount=\"7\" size=\"120\""
    " downloadId=\"download-other-book-id\" />\n"
    "</library>\n"
  );

  // "book1-id" belonged to the previously set library and is no longer known.
  EXPECT_THROW(dumper.dumpLibXMLContent({"book1-id"}), std::out_of_range);
}

TEST_F(LibXMLDumperTest, emptyBookmarksProducesEmptyBookmarksElement)
{
  EXPECT_EQ(dumper.dumpLibXMLBookmark(), "<bookmarks />\n");
}

TEST_F(LibXMLDumperTest, dumpsBookmarkUsingLiveBookDataWhenBookIsInLibrary)
{
  Bookmark bookmark;
  bookmark.setBookId("book1-id");
  bookmark.setUrl("/A/Some_Article");
  bookmark.setTitle("Some Article Title");
  lib->addBookmark(bookmark);

  // These values come from the live library entry, not from the bookmark's
  // own (unset) book* fields.
  EXPECT_EQ(dumper.dumpLibXMLBookmark(),
    "<bookmarks>\n"
    "  <bookmark>\n"
    "    <book>\n"
    "      <id>book1-id</id>\n"
    "      <title>First Book</title>\n"
    "      <name>test_book1-id</name>\n"
    "      <flavour>nopic</flavour>\n"
    "      <language>eng,spa</language>\n"
    "      <date>2021-03-25</date>\n"
    "    </book>\n"
    "    <title>Some Article Title</title>\n"
    "    <url>/A/Some_Article</url>\n"
    "  </bookmark>\n"
    "</bookmarks>\n"
  );
}

TEST_F(LibXMLDumperTest, dumpsBookmarkUsingStoredFieldsWhenBookIsUnknown)
{
  Bookmark bookmark;
  bookmark.setBookId("no-such-book");
  bookmark.setBookTitle("Vanished Book");
  bookmark.setBookName("vanished_book");
  bookmark.setBookFlavour("full");
  bookmark.setLanguage("fra");
  bookmark.setDate("2019-01-01");
  bookmark.setUrl("/A/Some_Article");
  bookmark.setTitle("Some Article Title");
  lib->addBookmark(bookmark);

  EXPECT_EQ(dumper.dumpLibXMLBookmark(),
    "<bookmarks>\n"
    "  <bookmark>\n"
    "    <book>\n"
    "      <id>no-such-book</id>\n"
    "      <title>Vanished Book</title>\n"
    "      <name>vanished_book</name>\n"
    "      <flavour>full</flavour>\n"
    "      <language>fra</language>\n"
    "      <date>2019-01-01</date>\n"
    "    </book>\n"
    "    <title>Some Article Title</title>\n"
    "    <url>/A/Some_Article</url>\n"
    "  </bookmark>\n"
    "</bookmarks>\n"
  );
}

}  // unnamed namespace
