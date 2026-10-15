#include "gtest/gtest.h"
#include "../include/manager.h"
#include "../include/library.h"
#include "../include/book.h"
#include "../include/tools.h"
#include "../src/tools/otherTools.h"
#include "testing_tools.h"
#include <fstream>

namespace
{

std::string resolveAbsPath(const std::string& basePath, const std::string& relPath)
{
    return kiwix::computeAbsolutePath(kiwix::removeLastPathElement(basePath), relPath);
}

// Absolute path of test/library.opds, computed (rather than hardcoded) so
// it resolves correctly regardless of the checkout location, since
// readFile() actually opens it.
// Built from single, separator-free path segments (rather than a
// "test/library.opds"-style literal) because computeAbsolutePath() splits
// on the OS-native separator only, so a literal using the "wrong" slash
// would silently fail to be split into components on Windows.
const std::string LIB_OPDS_ABS_PATH
    = kiwix::computeAbsolutePath(
        kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), "test"),
        "library.opds");

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
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), url);
}

TEST(ManagerTest, addBookFromPathAndGetIdWithAcquisitionUrlsTest)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager = kiwix::Manager(lib);

    const std::string zimUrl = "http://example.org/book.zim";
    const std::string meta4Url = "http://example.org/book.zim.meta4";
    const std::string torrentUrl = "http://example.org/book.zim.torrent";
    kiwix::Book::AcquisitionLinks urls;
    urls[static_cast<size_t>(kiwix::Book::AcquisitionLinkKind::DIRECT)] = zimUrl;
    urls[static_cast<size_t>(kiwix::Book::AcquisitionLinkKind::META4)] = meta4Url;
    urls[static_cast<size_t>(kiwix::Book::AcquisitionLinkKind::BITTORRENT)] = torrentUrl;
    auto bookId = manager.addBookFromPathAndGetId("./test/example.zim", "", urls);
    ASSERT_NE(bookId, "");
    kiwix::Book book = lib->getBookById(bookId);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), zimUrl);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::META4), meta4Url);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::BITTORRENT), torrentUrl);

    // Link kinds that were not provided are reported as empty URLs.
    const std::string otherZimUrl = "http://example.org/other.zim";
    kiwix::Book::AcquisitionLinks zimOnlyUrl;
    zimOnlyUrl[static_cast<size_t>(kiwix::Book::AcquisitionLinkKind::DIRECT)] = otherZimUrl;
    bookId = manager.addBookFromPathAndGetId("./test/example.zim", "", zimOnlyUrl);
    ASSERT_NE(bookId, "");
    book = lib->getBookById(bookId);
    EXPECT_EQ(kiwix::nonEmptyAcquisitionLinksCount(book), 1u);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), otherZimUrl);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::MAGNET), "");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::META4), "");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::BITTORRENT), "");
}

TEST(ManagerTest, readFileSetsWritableLibraryPathEvenIfFileDoesNotExist)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    const std::string nonExistentPath
        = kiwix::computeAbsolutePath(
              kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), "test"),
              "does_not_exist.opds");

    EXPECT_FALSE(manager.readFile(nonExistentPath));
    EXPECT_EQ(manager.writableLibraryPath, nonExistentPath);

    const std::string pathToSave = "./relative.zim";
    auto bookId = manager.addBookFromPathAndGetId("./test/example.zim", pathToSave);
    ASSERT_NE(bookId, "");
    kiwix::Book book = lib->getBookById(bookId);
    auto savedPath = resolveAbsPath(nonExistentPath, pathToSave);
    EXPECT_EQ(book.getPath(), savedPath);
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

TEST(ManagerTest, reload)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  manager.reload({"./test/library.opds"});
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

  manager.reload({"./test/library.opds"});
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
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    EXPECT_TRUE(manager.readOpds(sampleOpdsFeed, "http://example.com", /*readOnly=*/true));

    EXPECT_TRUE(lib->getBookById("book1").readOnly());
    EXPECT_TRUE(lib->getBookById("book2").readOnly());
}



TEST(ManagerTest, readOpdsKeepsBookWithAcquisitionLinkOfUnknownType)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  const std::string feed = R"(
      <feed xmlns="http://www.w3.org/2005/Atom">
        <entry>
          <id>urn:uuid:book1</id>
          <title>Book One</title>
          <link rel="http://opds-spec.org/acquisition/open-access"
                type="application/x-some-future-format"
                href="https://example.com/book1.future" />
          <link rel="http://opds-spec.org/acquisition/open-access"
                type="application/x-zim"
                href="https://example.com/book1.zim" />
        </entry>
        <entry>
          <id>urn:uuid:book2</id>
          <title>Book Two</title>
        </entry>
      </feed>
    )";

  // A single link of an unknown type must neither make the feed unreadable
  // nor drop its book.
  kiwix::testing::CapturedStderr stderror;
  EXPECT_TRUE(manager.readOpds(feed, "http://example.com"));

  EXPECT_EQ(
    "Book 'book1': ignoring acquisition link 'https://example.com/book1.future':"
    " Unknown acquisition link type: application/x-some-future-format\n",
    std::string(stderror));

  EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{"book1", "book2"}));
  EXPECT_EQ(lib->getBookById("book1").getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT),
            "https://example.com/book1.zim");
}

TEST(ManagerTest, readOpdsWithInvalidLocalPath)
{
  auto lib = kiwix::Library::create();
  kiwix::Manager manager(lib);

  const std::string feed = R"(
      <feed xmlns="http://www.w3.org/2005/Atom">
        <entry>
          <id>urn:uuid:book1</id>
          <title>Book From OPDS</title>
          <link rel="http://opds-spec.org/acquisition/open-access" href="does-not-exist.zim" />
        </entry>
      </feed>
    )";

  EXPECT_TRUE(manager.readOpds(feed, "./test/library.opds", /*readOnly=*/false));

  kiwix::Book book = lib->getBookById("book1");
  EXPECT_FALSE(book.isPathValid());
  EXPECT_EQ(book.getTitle(), "Book From OPDS");
}

namespace
{

// Writes 'content' to a file in the test directory and deletes the file
// when going out of scope.
struct TempLibraryFile
{
    TempLibraryFile(const std::string& name, const std::string& content)
      : path(kiwix::computeAbsolutePath(
                kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), "test"),
                name))
    {
        std::ofstream(path) << content;
    }

    ~TempLibraryFile() { std::remove(path.c_str()); }

    const std::string path;
};

} // unnamed namespace

TEST(ManagerTest, readFileRejectsNonOpdsContent)
{
    // The library.xml format of libkiwix 14 and earlier is no longer
    // supported (kiwix/libkiwix#1377).
    const TempLibraryFile legacyLibraryXml("legacy_library.xml", R"(
<library version="20110515">
  <book id="raycharles"
        path="./zimfile_raycharles.zim"
        title="Ray Charles"
        language="eng"
        date="2020-03-31"
        size="556"/>
</library>
)");
    const TempLibraryFile notXml("not_a_library.txt",
                                 "this is definitely not xml content");

    for ( const auto& path : {legacyLibraryXml.path, notXml.path} ) {
        auto lib = kiwix::Library::create();
        kiwix::Manager manager(lib);

        EXPECT_FALSE(manager.readFile(path)) << path;
        EXPECT_TRUE(lib->getBooksIds().empty()) << path;
        EXPECT_THROW(manager.reload({path}), std::runtime_error) << path;
        EXPECT_TRUE(lib->getBooksIds().empty()) << path;
    }
}

TEST(ManagerTest, readFile)
{
    auto lib = kiwix::Library::create();
    kiwix::Manager manager(lib);

    EXPECT_TRUE(manager.readFile(LIB_OPDS_ABS_PATH));
    EXPECT_EQ(manager.writableLibraryPath, LIB_OPDS_ABS_PATH);

    EXPECT_EQ(lib->getBooksIds(), (kiwix::Library::BookIdCollection{
          "charlesray",
          "inaccessiblezim",
          "raycharles",
          "raycharles_uncategorized"
    }));

    kiwix::Book book = lib->getBookById("raycharles");

    EXPECT_EQ(book.getPath(), resolveAbsPath(LIB_OPDS_ABS_PATH, "./zimfile_raycharles.zim"));
    EXPECT_TRUE(book.isPathValid());

    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles.zim");
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

    EXPECT_EQ(book.getSize(), 556U*1024U);

    auto illustration = book.getIllustration(48);

    EXPECT_EQ(illustration->getMimeType(), "image/png");
    EXPECT_EQ(illustration->getWidth(), 48);
    EXPECT_EQ(illustration->getHeight(), 48);
    EXPECT_EQ(illustration->getUrl(), "https://example.com/favicon/raycharles.png");
    EXPECT_FALSE(book.readOnly());
}
