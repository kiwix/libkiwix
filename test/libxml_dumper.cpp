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
#include "gtest/gtest.h"

namespace
{

using namespace kiwix;

// Only the book fields that end up in a dumped bookmark are set.
Book createBook(const std::string& id, const std::string& title)
{
  Book book;
  book.setId(id);
  book.setTitle(title);
  book.setLanguage("eng,spa");
  book.setDate("2021-03-25");
  book.setName("test_" + id);
  book.setFlavour("nopic");
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
  }

  LibraryPtr lib;
  LibXMLDumper dumper;
};

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
