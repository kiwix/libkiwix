#include "gtest/gtest.h"
#include "../include/manager.h"
#include "../include/library.h"
#include "../include/book.h"
#include "../include/tools.h"
#include <pugixml.hpp>

namespace
{

// parseXmlDom()/parseOpdsDom() are protected - this subclass re-exposes them
// as public so they can be exercised directly, without going through the
// readXml()/readOpds() wrappers.
struct TestManager : public kiwix::Manager
{
  using kiwix::Manager::Manager;
  using kiwix::Manager::parseXmlDom;
  using kiwix::Manager::parseOpdsDom;
};

// XMLDoc helper needs to build a pugi::xml_document from a string before
// handing it to parseXmlDom/parseOpdsDo
struct XMLDoc : pugi::xml_document
{
  explicit XMLDoc(const std::string& xml)
  {
    load_buffer(xml.c_str(), xml.size());
  }
};

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

TEST(ManagerTest, parseXmlDomAddsAllBooksAndHonorsReadOnly)
{
    const XMLDoc doc(R"(
      <library version="1.0">
        <book id="book1" path="book1.zim" title="Book One"></book>
        <book id="book2" path="book2.zim" title="Book Two"></book>
      </library>
    )");

    {
      auto lib = kiwix::Library::create();
      TestManager manager(lib);
      EXPECT_TRUE(manager.parseXmlDom(doc, /*readOnly=*/false, LIB_ABS_PATH, /*trustLibrary=*/true));

      EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{"book1", "book2"}));
      EXPECT_FALSE(lib->getBookById("book1").readOnly());
      EXPECT_FALSE(lib->getBookById("book2").readOnly());
    }

    {
      auto lib = kiwix::Library::create();
      TestManager manager(lib);
      EXPECT_TRUE(manager.parseXmlDom(doc, /*readOnly=*/true, LIB_ABS_PATH, /*trustLibrary=*/true));

      EXPECT_TRUE(lib->getBookById("book1").readOnly());
      EXPECT_TRUE(lib->getBookById("book2").readOnly());
    }
}

TEST(ManagerTest, parseXmlDomWithNoBooksReturnsTrue)
{
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(R"(<library version="1.0"></library>)");

    EXPECT_TRUE(manager.parseXmlDom(doc, true, LIB_ABS_PATH, true));
    EXPECT_TRUE(lib->getBooksIds().empty());
}

TEST(ManagerTest, parseXmlDomWithTrustLibraryFalseAndInvalidPath)
{
    // With trustLibrary=false and a path that can't be opened as a ZIM,
    // readBookFromPath() fails and the book is added as-is, using the
    // metadata coming from the XML.
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(R"(
      <library version="1.0">
        <book id="book1" path="does-not-exist.zim" title="Book From XML"></book>
      </library>
    )");

    EXPECT_TRUE(manager.parseXmlDom(doc, true, "./test/lib.xml", /*trustLibrary=*/false));

    kiwix::Book book = lib->getBookById("book1");
    EXPECT_FALSE(book.isPathValid());
    EXPECT_EQ(book.getTitle(), "Book From XML");
}

TEST(ManagerTest, parseXmlDomWithTrustLibraryFalseAndValidPathAdoptsZimMetadata)
{
    // With trustLibrary=false and a path pointing to a real ZIM,
    // readBookFromPath() re-reads the ZIM's own metadata - including its
    // UUID, which becomes the book's id - discarding whatever the XML said.
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(R"(
      <library version="1.0">
        <book id="book1" path="example.zim" title="Stale title from XML"></book>
      </library>
    )");

    EXPECT_TRUE(manager.parseXmlDom(doc, true, "./test/lib.xml", /*trustLibrary=*/false));

    const auto ids = lib->getBooksIds();
    ASSERT_EQ(ids.size(), 1U);
    kiwix::Book book = lib->getBookById(ids[0]);
    EXPECT_TRUE(book.isPathValid());
    EXPECT_NE(book.getTitle(), "Stale title from XML");
}

const char sampleOpdsFeed[] = R"(
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <totalResults>2</totalResults>
  <startIndex>0</startIndex>
  <itemsPerPage>2</itemsPerPage>
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

TEST(ManagerTest, parseOpdsDomAddsEntriesAndParsesSearchMetadata)
{
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(sampleOpdsFeed);

    EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"", /*readOnly=*/false, /*trustLibrary=*/true));

    EXPECT_TRUE(manager.m_hasSearchResult);
    EXPECT_EQ(manager.m_totalBooks, 2U);
    EXPECT_EQ(manager.m_startIndex, 0U);
    EXPECT_EQ(manager.m_itemsPerPage, 2U);

    EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{"book1", "book2"}));
    EXPECT_EQ(lib->getBookById("book1").getUrl(), "https://example.com/book1.zim");
}

TEST(ManagerTest, parseOpdsDomWithoutSearchMetadata)
{
    // strtoull() on the empty string returned for missing <totalResults> /
    // <startIndex> / <itemsPerPage> elements yields 0 without throwing, so
    // the try/catch in parseOpdsDom() never actually observes an error here
    // - m_hasSearchResult ends up true, with all counters at 0.
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(R"(
      <feed xmlns="http://www.w3.org/2005/Atom">
        <entry>
          <id>urn:uuid:book1</id>
          <title>Book One</title>
        </entry>
      </feed>
    )");

    EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"", /*readOnly=*/false, /*trustLibrary=*/true));

    EXPECT_TRUE(manager.m_hasSearchResult);
    EXPECT_EQ(manager.m_totalBooks, 0U);
    EXPECT_EQ(manager.m_startIndex, 0U);
    EXPECT_EQ(manager.m_itemsPerPage, 0U);

    EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{"book1"}));
}

TEST(ManagerTest, parseOpdsDomWithNoEntriesReturnsTrue)
{
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(R"(<feed xmlns="http://www.w3.org/2005/Atom"></feed>)");

    EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"", /*readOnly=*/false, /*trustLibrary=*/true));
    EXPECT_TRUE(lib->getBooksIds().empty());
}

TEST(ManagerTest, parseOpdsDomHonorsReadOnly)
{
    const XMLDoc doc(sampleOpdsFeed);

    {
      auto lib = kiwix::Library::create();
      TestManager manager(lib);
      EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"", /*readOnly=*/false, /*trustLibrary=*/true));

      EXPECT_FALSE(lib->getBookById("book1").readOnly());
      EXPECT_FALSE(lib->getBookById("book2").readOnly());
    }

    {
      auto lib = kiwix::Library::create();
      TestManager manager(lib);
      EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"", /*readOnly=*/true, /*trustLibrary=*/true));

      EXPECT_TRUE(lib->getBookById("book1").readOnly());
      EXPECT_TRUE(lib->getBookById("book2").readOnly());
    }
}

TEST(ManagerTest, parseOpdsDomWithTrustLibraryFalseHasNoEffectWithoutSelfLink)
{
    // sampleOpdsFeed's entries carry no rel="self" link, so
    // Book::updateFromOpds() never sets book.getPath() (see
    // BookTest.updateFromOPDSTest in test/book.cpp). So even with
    // trustLibrary=false, parseOpdsDom()'s "!book.getPath().empty()" guard
    // is never true, readBookFromPath() is never invoked, and the book
    // keeps exactly the metadata coming from the OPDS entry.
    // Contrast with parseOpdsDomWithTrustLibraryFalseAndInvalidSelfPath /
    // ...AndValidSelfPathAdoptsZimMetadata below, where a rel="self" link
    // does make trustLibrary=false take effect.
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(sampleOpdsFeed);

    EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"./test/lib.xml", /*readOnly=*/false, /*trustLibrary=*/false));

    kiwix::Book book1 = lib->getBookById("book1");
    EXPECT_EQ(book1.getTitle(), "Book One");
    EXPECT_EQ(book1.getPath(), "");
    EXPECT_FALSE(book1.isPathValid());
}

TEST(ManagerTest, parseOpdsDomWithTrustLibraryFalseAndInvalidSelfPath)
{
    // With trustLibrary=false and a rel="self" link that can't be opened as
    // a ZIM, readBookFromPath() fails and the book is added as-is, using
    // the metadata coming from the OPDS entry (mirrors
    // ManagerTest.parseXmlDomWithTrustLibraryFalseAndInvalidPath above).
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(R"(
      <feed xmlns="http://www.w3.org/2005/Atom">
        <entry>
          <id>urn:uuid:book1</id>
          <title>Book From OPDS</title>
          <link rel="self" href="does-not-exist.zim" />
        </entry>
      </feed>
    )");

    EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"./test/lib.xml", /*readOnly=*/false, /*trustLibrary=*/false));

    kiwix::Book book = lib->getBookById("book1");
    EXPECT_FALSE(book.isPathValid());
    EXPECT_EQ(book.getTitle(), "Book From OPDS");
}

TEST(ManagerTest, parseOpdsDomWithTrustLibraryFalseAndValidSelfPathAdoptsZimMetadata)
{
    // With trustLibrary=false and a rel="self" link pointing to a real
    // ZIM, readBookFromPath() re-reads the ZIM's own metadata - including
    // its UUID, which becomes the book's id - discarding whatever the OPDS
    // entry said (mirrors
    // ManagerTest.parseXmlDomWithTrustLibraryFalseAndValidPathAdoptsZimMetadata
    // above). The relative href is resolved against libraryPath's directory
    // ("./test"), exactly like updateFromXml()'s "path" attribute.
    auto lib = kiwix::Library::create();
    TestManager manager(lib);

    const XMLDoc doc(R"(
      <feed xmlns="http://www.w3.org/2005/Atom">
        <entry>
          <id>urn:uuid:book1</id>
          <title>Stale title from OPDS</title>
          <link rel="self" href="example.zim" />
        </entry>
      </feed>
    )");

    EXPECT_TRUE(manager.parseOpdsDom(doc, "http://example.com", /*libraryPath=*/"./test/lib.xml", /*readOnly=*/false, /*trustLibrary=*/false));

    const auto ids = lib->getBooksIds();
    ASSERT_EQ(ids.size(), 1U);
    kiwix::Book book = lib->getBookById(ids[0]);
    EXPECT_TRUE(book.isPathValid());
    EXPECT_NE(book.getTitle(), "Stale title from OPDS");
}

TEST(ManagerTest, readFileDetectsXmlFormat)
{
    // readFile() sniffs the file content for a "<feed" substring to tell
    // OPDS files apart from XML library files (see detectFormat() in
    // manager.cpp). library.xml contains no such substring, so it's routed
    // through parseXmlDom(), which - unlike the OPDS branch - resolves
    // relative "path" attributes against the file's own path (readFile()'s
    // 3rd argument to parseXmlDom() is `path`, not an empty string).
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    EXPECT_TRUE(manager.readFile("./test/library.xml"));

    EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{
          "charlesray",
          "inaccessiblezim",
          "raycharles",
          "raycharles_uncategorized"
    }));

    kiwix::Book book = lib->getBookById("raycharles");
    EXPECT_EQ(book.getPath(),
              kiwix::computeAbsolutePath(kiwix::removeLastPathElement("./test/library.xml"),
                                          "./zimfile_raycharles.zim"));
    // readOnly defaults to true.
    EXPECT_TRUE(book.readOnly());
}

TEST(ManagerTest, readFileDetectsOpdsFormat)
{
    // library.opds contains a "<feed" element, so it's routed through
    // parseOpdsDom() instead. Note readFile() always passes an empty
    // urlHost to parseOpdsDom(), unlike readOpds() which forwards the
    // caller-supplied one - so any relative link in the feed is left as-is.
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    EXPECT_TRUE(manager.readFile("./test/library.opds", /*readOnly=*/false, /*trustLibrary=*/true));

    EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{
          "charlesray",
          "inaccessiblezim",
          "raycharles",
          "raycharles_uncategorized"
    }));

    // library.opds is the OPDS analogue of library.xml - same books, with
    // every XML attribute mapped to its corresponding OPDS element/link
    // (see the comment at the top of library.opds). Check the "raycharles"
    // entry field-by-field, the same way ManagerTest.readXml does for its
    // XML-sourced equivalent.
    kiwix::Book book = lib->getBookById("raycharles");

    // Like XML's "path" attribute, the OPDS rel="self" link's href is
    // resolved against the library file's own directory (readFile() passes
    // its own `path` as parseOpdsDom()'s libraryPath) - library.opds's
    // "./zimfile_raycharles.zim" self link resolves to the same file as
    // library.xml's "path" attribute of the same value.
    EXPECT_EQ(book.getPath(),
              kiwix::computeAbsolutePath(kiwix::removeLastPathElement("./test/library.opds"),
                                          "./zimfile_raycharles.zim"));
    EXPECT_TRUE(book.isPathValid());

    EXPECT_EQ(book.getUrl(), "https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles.zim");
    EXPECT_EQ(book.getTitle(), "Ray Charles");
    EXPECT_EQ(book.getDescription(), "Wikipedia articles about Ray Charles (not all of them but near to what an average newborn may find more than enough)");
    EXPECT_EQ(book.getCommaSeparatedLanguages(), "eng");
    EXPECT_EQ(book.getCreator(), "Wikipedia");
    EXPECT_EQ(book.getPublisher(), "Kiwix");
    EXPECT_EQ(book.getDate(), "2020-03-31");
    EXPECT_EQ(book.getName(), "wikipedia_en_ray_charles");
    EXPECT_EQ(book.getTags(), "public_tag_without_a_value;_private_tag_without_a_value;wikipedia;_category:wikipedia;_pictures:no;_videos:no;_details:no;_ftindex:yes");
    EXPECT_EQ(book.getArticleCount(), 284U);
    EXPECT_EQ(book.getMediaCount(), 2U);

    // Unlike XML's "size" (KiB, converted to bytes by updateFromXml()),
    // OPDS's acquisition link "length" is taken as-is, already in bytes -
    // library.opds was written with 556*1024 to match library.xml's
    // size="556".
    EXPECT_EQ(book.getSize(), 556U*1024U);

    auto illustration = book.getIllustration(48);
    EXPECT_EQ(illustration->mimeType, "image/png");
    // urlHost is "" for readFile(), so the relative thumbnail href is left
    // untouched.
    EXPECT_EQ(illustration->url, "/favicon/raycharles.png");

    EXPECT_FALSE(book.readOnly());
}

TEST(ManagerTest, readFileWithOpdsFormatHonorsReadOnly)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    EXPECT_TRUE(manager.readFile("./test/library.opds", /*readOnly=*/true, /*trustLibrary=*/true));

    for (const auto& id : lib->getBooksIds()) {
      EXPECT_TRUE(lib->getBookById(id).readOnly());
    }
}

class ManagerReloadTest : public ::testing::TestWithParam<std::string> {};

TEST_P(ManagerReloadTest, reload)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  manager.reload({ GetParam() });
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

  manager.reload({ GetParam() });
  EXPECT_EQ(lib->getBooksIds(), kiwix::Library::BookIdCollection({
        "charlesray",
        "inaccessiblezim",
        "raycharles",
        "raycharles_uncategorized"
  }));
}

INSTANTIATE_TEST_CASE_P(XmlAndOpds, ManagerReloadTest,
    ::testing::Values("./test/library.xml", "./test/library.opds"));
