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

#include <pugixml.hpp>

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

// Parses `xmlContent` and returns the resulting document. Fails the current
// test if the content is not well-formed XML.
pugi::xml_document parseXML(const std::string& xmlContent)
{
  pugi::xml_document doc;
  const auto result = doc.load_string(xmlContent.c_str());
  EXPECT_TRUE(result) << "XML content is not well-formed: "
                      << result.description() << "\n"
                      << xmlContent;
  return doc;
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
  const auto doc = parseXML(dumper.dumpLibXMLContent({}));

  const auto library = doc.select_node("/library").node();
  ASSERT_TRUE(library);
  EXPECT_STREQ("20110515", library.attribute("version").as_string());
  EXPECT_FALSE(library.child("book"));
}

TEST_F(LibXMLDumperTest, dumpsRequestedBookWithAllAttributes)
{
  const auto doc = parseXML(dumper.dumpLibXMLContent({"book1-id"}));

  const auto book = doc.select_node("/library/book").node();
  ASSERT_TRUE(book);
  EXPECT_STREQ("book1-id", book.attribute("id").as_string());
  // The book's path is stored relative to the dumper's base dir.
  EXPECT_STREQ("book1-id.zim", book.attribute("path").as_string());
  EXPECT_STREQ("First Book", book.attribute("title").as_string());
  EXPECT_STREQ("Description of First Book",
               book.attribute("description").as_string());
  EXPECT_STREQ("eng,spa", book.attribute("language").as_string());
  EXPECT_STREQ("First Book Creator", book.attribute("creator").as_string());
  EXPECT_STREQ("First Book Publisher", book.attribute("publisher").as_string());
  EXPECT_STREQ("test_book1-id", book.attribute("name").as_string());
  EXPECT_STREQ("nopic", book.attribute("flavour").as_string());
  EXPECT_STREQ("tag1;tag2", book.attribute("tags").as_string());
  EXPECT_STREQ("2021-03-25", book.attribute("date").as_string());
  EXPECT_STREQ("http://download.kiwix.org/zim/book1-id.zim",
               book.attribute("url").as_string());
  EXPECT_STREQ("42", book.attribute("articleCount").as_string());
  EXPECT_STREQ("7", book.attribute("mediaCount").as_string());
  // Size is stored on disk in KiB (bytes >> 10).
  EXPECT_STREQ("120", book.attribute("size").as_string());
  EXPECT_STREQ("download-book1-id", book.attribute("downloadId").as_string());
  EXPECT_FALSE(book.attribute("origId"));
}

TEST_F(LibXMLDumperTest, dumpsOnlyRequestedBooksInOrder)
{
  const auto doc = parseXML(dumper.dumpLibXMLContent({"book2-id", "book1-id"}));

  const auto library = doc.select_node("/library").node();
  const auto books = library.children("book");
  ASSERT_EQ(2, std::distance(books.begin(), books.end()));

  auto it = books.begin();
  EXPECT_STREQ("book2-id", it->attribute("id").as_string());
  ++it;
  EXPECT_STREQ("book1-id", it->attribute("id").as_string());
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

  const auto doc = parseXML(dumper.dumpLibXMLContent({"bare-book-id"}));
  const auto bookNode = doc.select_node("/library/book").node();
  ASSERT_TRUE(bookNode);

  EXPECT_FALSE(bookNode.attribute("url"));
  EXPECT_FALSE(bookNode.attribute("downloadId"));
  EXPECT_FALSE(bookNode.attribute("articleCount"));
  EXPECT_FALSE(bookNode.attribute("mediaCount"));
  EXPECT_FALSE(bookNode.attribute("size"));
}

TEST_F(LibXMLDumperTest, skipsReadOnlyBooks)
{
  Book readOnlyBook = createBook("readonly-book-id", "Read Only Book");
  readOnlyBook.setReadOnly(true);
  lib->addBook(readOnlyBook);

  const auto doc
      = parseXML(dumper.dumpLibXMLContent({"book1-id", "readonly-book-id"}));
  const auto library = doc.select_node("/library").node();
  const auto books = library.children("book");
  // The read-only book is entirely omitted from the dump, not just stripped
  // of its attributes.
  EXPECT_EQ(1, std::distance(books.begin(), books.end()));
}

TEST_F(LibXMLDumperTest, aliasBookWithOrigIdOnlyEmitsIdAndOrigId)
{
  Book aliasBook = createBook("alias-book-id", "Alias Book");
  aliasBook.setOrigId("book1-id");
  lib->addBook(aliasBook);

  const auto doc = parseXML(dumper.dumpLibXMLContent({"alias-book-id"}));
  const auto book = doc.select_node("/library/book").node();
  ASSERT_TRUE(book);

  EXPECT_STREQ("alias-book-id", book.attribute("id").as_string());
  EXPECT_STREQ("book1-id", book.attribute("origId").as_string());
  // Metadata that's only meaningful for the "real" entry is skipped for a
  // flavour alias - only date/url/counts/size/downloadId (below the
  // origId/title branch in the source) are still emitted regardless.
  EXPECT_FALSE(book.attribute("title"));
  EXPECT_FALSE(book.attribute("description"));
  EXPECT_FALSE(book.attribute("name"));
  EXPECT_STREQ("2021-03-25", book.attribute("date").as_string());
  EXPECT_STREQ("download-alias-book-id",
               book.attribute("downloadId").as_string());
}

TEST_F(LibXMLDumperTest, dumpLibXMLContentThrowsForUnknownBookId)
{
  // Unlike LibOPDSDumper::dumpOPDSContent (which silently skips book ids no
  // longer present in the library), LibXMLDumper::dumpLibXMLContent uses
  // Library::getBookById() directly and lets std::out_of_range propagate.
  EXPECT_THROW(dumper.dumpLibXMLContent({"no-such-book"}), std::out_of_range);
}

TEST_F(LibXMLDumperTest, defaultConstructedDumperHasNoLibrary)
{
  LibXMLDumper freshDumper;
  const auto doc = parseXML(freshDumper.dumpLibXMLContent({"book1-id"}));

  const auto library = doc.select_node("/library").node();
  ASSERT_TRUE(library);
  EXPECT_FALSE(library.child("book"));

  const auto bookmarksDoc = parseXML(freshDumper.dumpLibXMLBookmark());
  const auto bookmarks = bookmarksDoc.select_node("/bookmarks").node();
  ASSERT_TRUE(bookmarks);
  EXPECT_FALSE(bookmarks.child("bookmark"));
}

TEST_F(LibXMLDumperTest, setLibraryReplacesTheDumpedLibrary)
{
  auto otherLib = Library::create();
  otherLib->addBook(createBook("other-book-id", "Other Book"));

  dumper.setLibrary(otherLib.get());

  const auto doc = parseXML(dumper.dumpLibXMLContent({"other-book-id"}));
  const auto book = doc.select_node("/library/book").node();
  ASSERT_TRUE(book);
  EXPECT_STREQ("other-book-id", book.attribute("id").as_string());

  // "book1-id" belonged to the previously set library and is no longer known.
  EXPECT_THROW(dumper.dumpLibXMLContent({"book1-id"}), std::out_of_range);
}

TEST_F(LibXMLDumperTest, emptyBookmarksProducesEmptyBookmarksElement)
{
  const auto doc = parseXML(dumper.dumpLibXMLBookmark());
  const auto bookmarks = doc.select_node("/bookmarks").node();
  ASSERT_TRUE(bookmarks);
  EXPECT_FALSE(bookmarks.child("bookmark"));
}

TEST_F(LibXMLDumperTest, dumpsBookmarkUsingLiveBookDataWhenBookIsInLibrary)
{
  Bookmark bookmark;
  bookmark.setBookId("book1-id");
  bookmark.setUrl("/A/Some_Article");
  bookmark.setTitle("Some Article Title");
  lib->addBookmark(bookmark);

  const auto doc = parseXML(dumper.dumpLibXMLBookmark());
  const auto bookmarkNode = doc.select_node("/bookmarks/bookmark").node();
  ASSERT_TRUE(bookmarkNode);

  EXPECT_EQ("Some Article Title",
            std::string(bookmarkNode.child_value("title")));
  EXPECT_EQ("/A/Some_Article", std::string(bookmarkNode.child_value("url")));

  const auto bookNode = bookmarkNode.child("book");
  ASSERT_TRUE(bookNode);
  // These values come from the live library entry, not from the bookmark's
  // own (unset) book* fields.
  EXPECT_EQ("book1-id", std::string(bookNode.child_value("id")));
  EXPECT_EQ("First Book", std::string(bookNode.child_value("title")));
  EXPECT_EQ("test_book1-id", std::string(bookNode.child_value("name")));
  EXPECT_EQ("nopic", std::string(bookNode.child_value("flavour")));
  EXPECT_EQ("eng,spa", std::string(bookNode.child_value("language")));
  EXPECT_EQ("2021-03-25", std::string(bookNode.child_value("date")));
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

  const auto doc = parseXML(dumper.dumpLibXMLBookmark());
  const auto bookNode = doc.select_node("/bookmarks/bookmark/book").node();
  ASSERT_TRUE(bookNode);

  EXPECT_EQ("no-such-book", std::string(bookNode.child_value("id")));
  EXPECT_EQ("Vanished Book", std::string(bookNode.child_value("title")));
  EXPECT_EQ("vanished_book", std::string(bookNode.child_value("name")));
  EXPECT_EQ("full", std::string(bookNode.child_value("flavour")));
  EXPECT_EQ("fra", std::string(bookNode.child_value("language")));
  EXPECT_EQ("2019-01-01", std::string(bookNode.child_value("date")));
}

}  // unnamed namespace
