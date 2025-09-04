#include "gtest/gtest.h"
#include "../include/manager.h"
#include "../include/library.h"
#include "../include/book.h"
#include "../include/tools.h"
#include <iostream>
#include <fstream>

TEST(ManagerTest, addBookFromPathAndGetIdTest)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager = kiwix::Manager(lib);

    auto bookId = manager.addBookFromPathAndGetId("./test/example.zim");
    ASSERT_NE(bookId, "");
    kiwix::Book book = lib->getBookById(bookId);
    EXPECT_EQ(book.getPath(), kiwix::computeAbsolutePath("", "./test/example.zim"));

    const std::string pathToSave = "./pathToSave";
    const std::string url = "url";
    bookId = manager.addBookFromPathAndGetId("./test/example.zim", pathToSave, url, true);
    book = lib->getBookById(bookId);
    auto savedPath = kiwix::computeAbsolutePath(kiwix::removeLastPathElement(manager.writableLibraryPath), pathToSave);
    EXPECT_EQ(book.getPath(), savedPath);
    EXPECT_EQ(book.getUrl(), url);
}



#if _WIN32
# define UNITTEST_ZIM_PATH "zimfiles\\unittest.zim"
# define LIB_ABS_PATH  "C:\\data\\lib.xml"
# define ZIM_ABS_PATH  "C:\\data\\zimfiles\\unittest.zim"
#else
# define UNITTEST_ZIM_PATH "zimfiles/unittest.zim"
# define LIB_ABS_PATH "/data/lib.xml"
# define ZIM_ABS_PATH "/data/zimfiles/unittest.zim"
#endif

const char sampleLibraryXML[] = R"(
<library version="1.0">
  <book
        id="0d0bcd57-d3f6-cb22-44cc-a723ccb4e1b2"
        path=")" UNITTEST_ZIM_PATH R"("
        url="https://example.com/zimfiles/unittest.zim"
        title="Unit Test"
        description="Wikipedia articles about unit testing"
        language="eng"
        creator="Wikipedia"
        publisher="Kiwix"
        date="2020-03-31"
        name="wikipedia_en_unit_testing"
        tags="unittest;wikipedia"
        articleCount="123"
        mediaCount="45"
        size="678"
      ></book>
</library>
)";

TEST(ManagerTest, readXml)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager = kiwix::Manager(lib);

    EXPECT_EQ(true, manager.readXml(sampleLibraryXML, true, LIB_ABS_PATH, true));
    kiwix::Book book = lib->getBookById("0d0bcd57-d3f6-cb22-44cc-a723ccb4e1b2");
    EXPECT_EQ(ZIM_ABS_PATH, book.getPath());
    EXPECT_EQ("https://example.com/zimfiles/unittest.zim", book.getUrl());
    EXPECT_EQ("Unit Test", book.getTitle());
    EXPECT_EQ("Wikipedia articles about unit testing", book.getDescription());
    EXPECT_EQ("eng", book.getCommaSeparatedLanguages());
    EXPECT_EQ("Wikipedia", book.getCreator());
    EXPECT_EQ("Kiwix", book.getPublisher());
    EXPECT_EQ("2020-03-31", book.getDate());
    EXPECT_EQ("wikipedia_en_unit_testing", book.getName());
    EXPECT_EQ("unittest;wikipedia", book.getTags());
    EXPECT_EQ(123U, book.getArticleCount());
    EXPECT_EQ(45U, book.getMediaCount());
    EXPECT_EQ(678U*1024, book.getSize());
}

TEST(Manager, reload)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  manager.reload({ "./test/library.xml" });
  EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{
        "charlesray",
        "inaccessiblezim",
        "raycharles",
        "raycharles_uncategorized"
  }));

  lib->removeBookById("raycharles");
  EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{
        "charlesray",
        "inaccessiblezim",
        "raycharles_uncategorized"
  }));

  manager.reload({ "./test/library.xml" });
  EXPECT_EQ(lib->getBooksIds(), kiwix::Library::BookIdCollection({
        "charlesray",
        "inaccessiblezim",
        "raycharles",
        "raycharles_uncategorized"
  }));
}
