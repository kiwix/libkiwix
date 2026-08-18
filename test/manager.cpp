#include "gtest/gtest.h"
#include "../include/manager.h"
#include "../include/library.h"
#include "../include/book.h"
#include "../include/tools.h"
#include <iostream>
#include <fstream>

namespace
{

std::string resolveAbsPath(const std::string& basePath, const std::string& relPath)
{
    return kiwix::computeAbsolutePath(kiwix::removeLastPathElement(basePath), relPath);
}

} // unnamed namespace

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
    auto savedPath = resolveAbsPath(manager.writableLibraryPath, pathToSave);
    EXPECT_EQ(book.getPath(), savedPath);
    EXPECT_EQ(book.getUrl(), url);
}

TEST(ManagerTest, readFileSetsWritableLibraryPathEvenIfFileDoesNotExist)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    const std::string nonExistentPath
        = kiwix::computeAbsolutePath(
              kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), "test"),
              "does_not_exist.xml");

    EXPECT_FALSE(manager.readFile(nonExistentPath, /*readOnly=*/false));
    EXPECT_EQ(manager.writableLibraryPath, nonExistentPath);

    const std::string pathToSave = "./relative.zim";
    auto bookId = manager.addBookFromPathAndGetId("./test/example.zim", pathToSave);
    ASSERT_NE(bookId, "");
    kiwix::Book book = lib->getBookById(bookId);
    auto savedPath = resolveAbsPath(nonExistentPath, pathToSave);
    EXPECT_EQ(book.getPath(), savedPath);
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

    // "path" is relative in the XML - readXml() resolves it against the
    // directory of LIB_ABS_PATH, yielding ZIM_ABS_PATH.
    EXPECT_EQ(ZIM_ABS_PATH, book.getPath());
    // ... but ZIM_ABS_PATH doesn't exist on disk, so the resolved path is
    // not considered valid.
    EXPECT_FALSE(book.isPathValid());
    EXPECT_TRUE(book.readOnly());
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

TEST(ManagerTest, readXmlInvalid)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager = kiwix::Manager(lib);

  const std::string invalidXML = R"(
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
  <book
        id="1a1bcd57-d3f6-cb22-44cc-a723ccb4e1b3"
        url="https://example.com/zimfiles/unittest2.zim"
        title="Unit Test 2"
)";

  EXPECT_FALSE(manager.readXml(invalidXML, true, LIB_ABS_PATH, true));
  EXPECT_TRUE(lib->getBooksIds().empty());
}

TEST(ManagerTest, readXmlNotXml)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager = kiwix::Manager(lib);

    const std::string notXML = "this is definitely not xml content";

    EXPECT_FALSE(manager.readXml(notXML, true, LIB_ABS_PATH, true));
    EXPECT_TRUE(lib->getBooksIds().empty());
}

TEST(ManagerTest, readOpdsWithNoEntriesReturnsTrue)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  EXPECT_TRUE(manager.readOpds(R"(<feed xmlns="http://www.w3.org/2005/Atom"></feed>)", "http://example.com"));
  EXPECT_TRUE(lib->getBooksIds().empty());
}

TEST(ManagerTest, readOpdsWithMalformedInputAddsNoBooks)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  const std::string feed = R"(
      <feed xmlns="http://www.w3.org/2005/Atom">
        <entry>
          <id>urn:uuid:book1</id>
          <title>Book One</title>
        </entry>
        <entry>
          <id>urn:uuid:book2</id>
          <title>Book Two</title>
      </feed>
    )";

  EXPECT_FALSE(manager.readOpds(feed, "http://example.com"));
  EXPECT_TRUE(lib->getBooksIds().empty());
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

const char sampleOpdsFeed[] = R"(
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <totalResults>9</totalResults>
  <startIndex>7</startIndex>
  <itemsPerPage>10</itemsPerPage>
  <entry>
    <id>urn:uuid:book1</id>
    <title>Book One</title>
    <link rel="http://opds-spec.org/acquisition/open-access"
          type="application/x-zim"
          href="https://example.com/book1.zim"
          length="111" />
  </entry>
  <entry>
    <id>urn:uuid:book2</id>
    <title>Book Two</title>
    <link rel="http://opds-spec.org/acquisition/open-access"
          type="application/x-zim"
          href="https://example.com/book2.zim"
          length="222" />
  </entry>
</feed>
)";

TEST(ManagerTest, readOpdsHonorsReadOnlyTrue)
{
    // readOpds() defaults to readOnly=false (see
    // ManagerTest.readOpdsAddsEntriesAndParsesSearchMetadata below, which
    // covers that case) - this checks that readOnly=true is honored too.
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    EXPECT_TRUE(manager.readOpds(sampleOpdsFeed, "http://example.com", /*readOnly=*/true));

    EXPECT_TRUE(lib->getBookById("book1").readOnly());
    EXPECT_TRUE(lib->getBookById("book2").readOnly());
}

TEST(ManagerTest, readOpdsAddsEntriesAndParsesSearchMetadata)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    EXPECT_TRUE(manager.readOpds(sampleOpdsFeed, "http://example.com"));

    EXPECT_TRUE(manager.m_hasSearchResult);
    EXPECT_EQ(manager.m_totalBooks, 9U);
    EXPECT_EQ(manager.m_startIndex, 7U);
    EXPECT_EQ(manager.m_itemsPerPage, 10U);

    EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{"book1", "book2"}));

    kiwix::Book book1 = lib->getBookById("book1");
    EXPECT_EQ(book1.getTitle(), "Book One");
    EXPECT_EQ(book1.getUrl(), "https://example.com/book1.zim");
    // OPDS entries carry no local path at this point (unlike XML's "path"
    // attribute - see ManagerTest.readXml above): this feed has no
    // rel="self" link, so the resolved path stays empty and invalid.
    EXPECT_EQ(book1.getPath(), "");
    EXPECT_FALSE(book1.isPathValid());
    // Unlike readXml() (readOnly defaults to true), OPDS-sourced books are
    // always mutable.
    EXPECT_FALSE(book1.readOnly());
}

TEST(ManagerTest, readOpdsWithoutSearchMetadata)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  const std::string feed = R"(
      <feed xmlns="http://www.w3.org/2005/Atom">
        <entry>
          <id>urn:uuid:book1</id>
          <title>Book One</title>
        </entry>
      </feed>
    )";

  EXPECT_TRUE(manager.readOpds(feed, "http://example.com"));

  // None of <totalResults>/<startIndex>/<itemsPerPage> are present, so
  // there's no search result to report.
  EXPECT_FALSE(manager.m_hasSearchResult);
  EXPECT_EQ(manager.m_totalBooks, 0U);
  EXPECT_EQ(manager.m_startIndex, 0U);
  EXPECT_EQ(manager.m_itemsPerPage, 0U);

  EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{"book1"}));
}