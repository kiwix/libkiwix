#include "gtest/gtest.h"
#include "../include/book.h"
#include "testing_tools.h"
#include <pugixml.hpp>
#include <zim/archive.h>

namespace
{

struct XMLDoc : pugi::xml_document
{
  explicit XMLDoc(const std::string& xml)
  {
    load_buffer(xml.c_str(), xml.size());
  }
};

} // unnamed namespace

#if _WIN32
# define DATA_ABS_PATH "C:\\data\\zim"
# define ZARA_ABS_PATH "C:\\data\\zim\\zara.zim"
#else
# define DATA_ABS_PATH "/data/zim"
# define ZARA_ABS_PATH "/data/zim/zara.zim"
#endif

TEST(BookTest, updateFromXMLTest)
{
    const XMLDoc xml(R"(
      <book id="zara"
            path="zara.zim"
            url="https://who.org/zara.zim"
            title="Catch an infection in 24 hours"
            description="Complete guide to contagious diseases"
            language="eng"
            creator="World Health Organization"
            publisher="WHO"
            date="2020-03-31"
            name="who_contagious_diseases_en"
            tags="unittest;_category:medicine;_pictures:yes"
            articleCount="123456"
            mediaCount="234567"
            size="345678"
            favicon="ZmFrZS1ib29rLWZhdmljb24tZGF0YQ=="
            faviconMimeType="text/plain"
            faviconUrl="http://who.org/zara.fav"
          >
      </book>
    )");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), DATA_ABS_PATH);

    EXPECT_EQ(book.getPath(), ZARA_ABS_PATH);
    EXPECT_EQ(book.getUrl(), "https://who.org/zara.zim");
    EXPECT_EQ(book.getTitle(), "Catch an infection in 24 hours");
    EXPECT_EQ(book.getDescription(), "Complete guide to contagious diseases");
    EXPECT_EQ(book.getTags(), "unittest;_category:medicine;_pictures:yes");
    EXPECT_EQ(book.getName(), "who_contagious_diseases_en");
    EXPECT_EQ(book.getCategory(), "medicine");
    EXPECT_EQ(book.getArticleCount(), 123456U);
    EXPECT_EQ(book.getMediaCount(), 234567U);
    EXPECT_EQ(book.getSize(), 345678U*1024U);
    auto defaultIllustration = book.getIllustration(48);
    EXPECT_EQ(defaultIllustration->getData(), "fake-book-favicon-data");
    EXPECT_EQ(defaultIllustration->mimeType, "text/plain");
    EXPECT_EQ(defaultIllustration->url, "http://who.org/zara.fav");
}

// OPDS analogue of updateFromXMLTest above.
TEST(BookTest, updateFromOPDSTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <title>Catch an infection in 24 hours</title>
        <summary>Complete guide to contagious diseases</summary>
        <name>who_contagious_diseases_en</name>
        <tags>unittest;_category:medicine;_pictures:yes</tags>
        <category>medicine</category>
        <articleCount>123456</articleCount>
        <mediaCount>234567</mediaCount>
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="https://who.org/zara.zim"
              length="345678" />
        <link rel="http://opds-spec.org/image/thumbnail"
              type="text/plain"
              href="/zara.fav" />
      </entry>
    )");

    kiwix::Book book;
    book.updateFromOpds(opds.child("entry"), "http://who.org", "");

    EXPECT_EQ(book.getPath(), "");
    EXPECT_FALSE(book.isPathValid());

    // The "urn:uuid:" prefix (as used by <id> in real OPDS feeds) must be
    // stripped.
    EXPECT_EQ(book.getId(), "zara");

    EXPECT_EQ(book.getUrl(), "https://who.org/zara.zim");
    EXPECT_EQ(book.getTitle(), "Catch an infection in 24 hours");
    EXPECT_EQ(book.getDescription(), "Complete guide to contagious diseases");
    EXPECT_EQ(book.getTags(), "unittest;_category:medicine;_pictures:yes");
    EXPECT_EQ(book.getName(), "who_contagious_diseases_en");
    EXPECT_EQ(book.getCategory(), "medicine");
    EXPECT_EQ(book.getArticleCount(), 123456U);
    EXPECT_EQ(book.getMediaCount(), 234567U);

    // Unlike updateFromXml()'s "size" attribute (interpreted in KiB and
    // converted to bytes), the OPDS acquisition link's "length" is taken
    // as-is, already in bytes.
    EXPECT_EQ(book.getSize(), 345678U);

    // Unlike updateFromXml()'s "favicon" attribute (which embeds the
    // actual base64-encoded image data directly), OPDS only ever gives us
    // a URL to fetch the thumbnail from later, so there's no embedded data
    // to check here - just that the URL (prefixed with urlHost) and mime
    // type made it through.
    auto defaultIllustration = book.getIllustration(48);
    EXPECT_EQ(defaultIllustration->mimeType, "text/plain");
    EXPECT_EQ(defaultIllustration->url, "http://who.org/zara.fav");
}

TEST(BookTest, updateFromOPDSLocalPathAcquisitionLinkTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <link rel="http://opds-spec.org/acquisition/open-access" href="zara.zim" />
      </entry>
    )");

    kiwix::Book book;
    book.updateFromOpds(opds.child("entry"), "http://who.org", DATA_ABS_PATH);

    EXPECT_EQ(book.getPath(), ZARA_ABS_PATH);
    EXPECT_EQ(book.getUrl(), "");
}

TEST(BookTest, updateFromOPDSTwoAcquisitionLinksTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="zara.zim"
              length="111" />
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="https://who.org/zara.zim"
              length="222" />
      </entry>
    )");

    kiwix::Book book;
    book.updateFromOpds(opds.child("entry"), "http://who.org", DATA_ABS_PATH);

    EXPECT_EQ(book.getPath(), ZARA_ABS_PATH);
    EXPECT_EQ(book.getUrl(), "https://who.org/zara.zim");
    EXPECT_EQ(book.getSize(), 222U);
}

TEST(BookTest, updateFromOPDSDuplicateLengthWarnsTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="zara.zim"
              length="111" />
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="https://who.org/zara.zim"
              length="222" />
      </entry>
    )");

    kiwix::Book book;
    kiwix::testing::CapturedStderr stderror;
    book.updateFromOpds(opds.child("entry"), "http://who.org", DATA_ABS_PATH);

    EXPECT_EQ(
      "Book 'zara': acquisition links 'zara.zim' (length 111) and "
      "'https://who.org/zara.zim' (length 222) disagree on length.\n",
      std::string(stderror));
    EXPECT_EQ(book.getSize(), 222U);
}

TEST(BookTest, setUrlWithMimeTypeAddsToAcquisitionLinks)
{
    kiwix::Book book;
    book.setUrl("http://who.org/zara.zim");
    book.setUrl("application/metalink4+xml", "http://who.org/zara.zim.meta4");
    book.setUrl("application/x-bittorrent", "http://who.org/zara.zim.torrent");

    const auto& links = book.getAcquisitionLinks();
    ASSERT_EQ(links.size(), 3U);
    EXPECT_EQ(links[0].mimeType, kiwix::Book::ACQUISITION_MIMETYPE_ZIM);
    EXPECT_EQ(links[0].url, "http://who.org/zara.zim");
    EXPECT_EQ(links[1].mimeType, "application/metalink4+xml");
    EXPECT_EQ(links[1].url, "http://who.org/zara.zim.meta4");
    EXPECT_EQ(links[2].mimeType, "application/x-bittorrent");
    EXPECT_EQ(links[2].url, "http://who.org/zara.zim.torrent");
}

TEST(BookTest, getUrlIgnoresNonZimAcquisitionLinks)
{
    kiwix::Book book;
    book.setUrl("application/metalink4+xml", "http://who.org/zara.zim.meta4");
    book.setUrl("application/x-bittorrent", "http://who.org/zara.zim.torrent");

    EXPECT_EQ(book.getUrl(), "");

    book.setUrl("http://who.org/zara.zim");
    book.setUrl("application/x-bittorrent", "http://who.org/zara.zim.torrent");

    EXPECT_EQ(book.getUrl(), "http://who.org/zara.zim");
}

namespace
{

kiwix::Book makeBook(const std::string& attr, const std::string& baseDir="")
{
    const XMLDoc xml("<book " + attr + "></book>");
    kiwix::Book book;
    book.updateFromXml(xml.child("book"), baseDir);
    return book;
}

kiwix::Book makeBookFromOpds(const std::string& entryContent, const std::string& urlHost="", const std::string& baseDir="")
{
    const XMLDoc opds("<entry>" + entryContent + "</entry>");
    kiwix::Book book;
    book.updateFromOpds(opds.child("entry"), urlHost, baseDir);
    return book;
}

} // unnamed namespace

TEST(BookTest, updateFromXMLCategoryHandlingTest)
{
  {
    const kiwix::Book book = makeBook(R"(
        id="abcd"
        tags="_category:category_defined_via_tags_only"
    )");

    EXPECT_EQ(book.getCategory(), "category_defined_via_tags_only");
  }
  {
    const kiwix::Book book = makeBook(R"(
        id="abcd"
        category="category_defined_via_attribute_only"
    )");

    EXPECT_EQ(book.getCategory(), "category_defined_via_attribute_only");
  }
  {
    const kiwix::Book book = makeBook(R"(
        id="abcd"
        category="category_attribute_overrides_tags"
        tags="_category:tags_override_category_attribute"
    )");

    EXPECT_EQ(book.getCategory(), "category_attribute_overrides_tags");
  }
  {
    const kiwix::Book book = makeBook(R"(
        id="abcd"
        tags="_category:tags_override_category_attribute"
        category="category_attribute_overrides_tags"
    )");

    EXPECT_EQ(book.getCategory(), "category_attribute_overrides_tags");
  }
}

TEST(BookTest, updateFromOPDSCategoryHandlingTest)
{
  {
    const kiwix::Book book = makeBookFromOpds(R"(
        <id>abcd</id>
        <tags>_category:category_defined_via_tags_only</tags>
    )");

    EXPECT_EQ(book.getCategory(), "category_defined_via_tags_only");
  }
  {
    const kiwix::Book book = makeBookFromOpds(R"(
        <id>abcd</id>
        <category>category_defined_via_element_only</category>
    )");

    EXPECT_EQ(book.getCategory(), "category_defined_via_element_only");
  }
  {
    const kiwix::Book book = makeBookFromOpds(R"(
        <id>abcd</id>
        <category>category_element_overrides_tags</category>
        <tags>_category:tags_override_category_element</tags>
    )");

    EXPECT_EQ(book.getCategory(), "category_element_overrides_tags");
  }
  {
    const kiwix::Book book = makeBookFromOpds(R"(
        <id>abcd</id>
        <tags>_category:tags_override_category_element</tags>
        <category>category_element_overrides_tags</category>
    )");

    EXPECT_EQ(book.getCategory(), "category_element_overrides_tags");
  }
}

TEST(BookTest, updateFromOPDSThumbnailWithAbsoluteHrefIgnoresUrlHostTest)
{
    // An already-absolute href (as real OPDS catalogs commonly send, see
    // test/data/library.opds) must be left untouched, not prefixed with
    // urlHost - concatenating the two would produce a garbled URL. This is
    // the counterpart to updateFromOPDSTest above, which covers the
    // relative-href case (where prefixing with urlHost IS expected).
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png"
              href="https://example.com/favicon/zara.png" />
    )", "http://who.org");

    const auto illustration = book.getIllustrations().at(0);
    EXPECT_EQ(illustration->url, "https://example.com/favicon/zara.png");
}

TEST(BookTest, updateFromOPDSThumbnailLinkTypeWithSizeSuffixTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png;width=96;height=256;scale=1"
              href="https://example.com/zara.png" />
    )");

    const auto illustration = book.getIllustrations().at(0);
    EXPECT_EQ(illustration->mimeType, "image/png");
    EXPECT_EQ(illustration->width, 96);
    EXPECT_EQ(illustration->height, 256);
}

TEST(BookTest, updateFromOPDSThumbnailLinkTypeWithPartialSizeSuffixTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png;width=96"
              href="https://example.com/zara.png" />
    )");

    const auto illustration = book.getIllustrations().at(0);
    EXPECT_EQ(illustration->mimeType, "image/png");
    EXPECT_EQ(illustration->width, 96);
    EXPECT_EQ(illustration->height, 48);
}

TEST(BookTest, updateFromOPDSDataUriThumbnailTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/jpeg;width=96;height=256;scale=1"
              href="data:image/jpeg;base64,Zmlyc3QtdGh1bWJuYWls" />
    )");

    const auto illustration = book.getIllustrations().at(0);
    EXPECT_EQ(illustration->getData(), "first-thumbnail");
    EXPECT_EQ(illustration->url, "");
    EXPECT_EQ(illustration->mimeType, "image/jpeg");
    EXPECT_EQ(illustration->width, 96);
    EXPECT_EQ(illustration->height, 256);
}

TEST(BookTest, updateFromOPDSMultipleDataUriThumbnailsTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png;width=48;height=48;scale=1"
              href="data:image/png;base64,Zmlyc3QtdGh1bWJuYWls" />
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/jpeg;width=96;height=96;scale=1"
              href="data:image/jpeg;base64,c2Vjb25kLXRodW1ibmFpbA==" />
    )");

    const auto& illustrations = book.getIllustrations();
    ASSERT_EQ(illustrations.size(), 2U);
    EXPECT_EQ(illustrations[0]->getData(), "first-thumbnail");
    EXPECT_EQ(illustrations[0]->width, 48);
    EXPECT_EQ(illustrations[0]->height, 48);
    EXPECT_EQ(illustrations[1]->getData(), "second-thumbnail");
    EXPECT_EQ(illustrations[1]->width, 96);
    EXPECT_EQ(illustrations[1]->height, 96);
}

TEST(BookTest, updateFromOPDSMixedDataUriAndExternalThumbnailLinksTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png"
              href="https://example.com/favicon.png" />
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/jpeg"
              href="data:image/jpeg;base64,Zmlyc3QtdGh1bWJuYWls" />
    )");

    const auto& illustrations = book.getIllustrations();
    ASSERT_EQ(illustrations.size(), 2U);
    EXPECT_EQ(illustrations[0]->url, "https://example.com/favicon.png");
    EXPECT_EQ(illustrations[1]->url, "");
    EXPECT_EQ(illustrations[1]->getData(), "first-thumbnail");
}

TEST(BookTest, updateFromOPDSDataUriThumbnailWithoutCommaTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png"
              href="data:image/png;base64" />
    )");

    const auto illustration = book.getIllustrations().at(0);
    EXPECT_EQ(illustration->getData(), "");
    EXPECT_EQ(illustration->url, "");
}

TEST(BookTest, updateFromOPDSThumbnailLinkWithoutTypeIsIgnoredTest)
{
  const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              href="https://example.com/favicon.png" />
    )");

  EXPECT_TRUE(book.getIllustrations().empty());
}

TEST(BookTest, updateFromOPDSNoThumbnailsTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <id>abcd</id>
    )");

    EXPECT_TRUE(book.getIllustrations().empty());
}

TEST(BookTest, setTagsDoesntAffectCategory)
{
    kiwix::Book book;

    book.setTags("_category:youtube");
    ASSERT_EQ("", book.getCategory());
}

TEST(BookTest, updateCopiesCategory)
{
    const kiwix::Book book = makeBook(R"(id="abcd" category="ted")");

    kiwix::Book newBook;
    newBook.setId("abcd");
    EXPECT_EQ(newBook.getCategory(), "");
    newBook.update(book);
    EXPECT_EQ(newBook.getCategory(), "ted");
}

TEST(BookTest, updateTest)
{
    kiwix::Book book = makeBook(R"(
        id="xyz"
        path="/home/user/Downloads/skin-of-color-society_en_all_2019-11.zim"
        url="book-url"
        name="skin-of-color-society_en_all"
        tags="youtube;_videos:yes;_ftindex:yes;_ftindex:yes;_pictures:yes;_details:yes"
        favicon="Ym9vay1mYXZpY29u"
        faviconMimeType="book-favicon-mimetype"
    )", "/data/zim");

    book.setReadOnly(false);
    book.setPathValid(true);

    kiwix::Book newBook;

    newBook.setReadOnly(true);
    EXPECT_FALSE(newBook.update(book));

    newBook.setReadOnly(false);
    EXPECT_FALSE(newBook.update(book));

    newBook.setId("xyz");
    EXPECT_TRUE(newBook.update(book));

    EXPECT_EQ(newBook.readOnly(), book.readOnly());
    EXPECT_EQ(newBook.getPath(), book.getPath());
    EXPECT_EQ(newBook.isPathValid(), book.isPathValid());
    EXPECT_EQ(newBook.getUrl(), book.getUrl());
    EXPECT_EQ(newBook.getTags(), book.getTags());
    EXPECT_EQ(newBook.getCategory(), book.getCategory());
    EXPECT_EQ(newBook.getName(), book.getName());
    auto defaultIllustration = book.getIllustration(48);
    auto newDefaultIllustration = newBook.getIllustration(48);
    EXPECT_EQ(newDefaultIllustration->getData(), defaultIllustration->getData());
    EXPECT_EQ(newDefaultIllustration->mimeType, defaultIllustration->mimeType);
}

TEST(BookTest, updateFromArchiveSetsByteExactSize)
{
    const zim::Archive archive("./test/zimfile.zim");
    kiwix::Book book;
    book.update(archive);
    EXPECT_EQ(book.getSize(), archive.getFilesize());
}

namespace
{

std::string path2HumanReadableId(const std::string& path)
{
    const XMLDoc xml("<book id=\"xyz\" path=\"" + path + "\"></book>");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), "/data/zim");
    return book.getHumanReadableIdFromPath();
}

} // unnamed namespace

TEST(BookTest, getHumanReadableIdFromPath)
{
  EXPECT_EQ("abc",     path2HumanReadableId("abc.zim"));
  EXPECT_EQ("abc",     path2HumanReadableId("ABC.zim"));
  EXPECT_EQ("abc",     path2HumanReadableId("âbç.zim"));
  EXPECT_EQ("ancient", path2HumanReadableId("ancient.zimbabwe"));
  EXPECT_EQ("ab_cd",   path2HumanReadableId("ab cd.zim"));
#ifdef _WIN32
  EXPECT_EQ("abc",     path2HumanReadableId("C:\\Data\\ZIM\\abc.zim"));
#else
  EXPECT_EQ("abc",     path2HumanReadableId("/Data/ZIM/abc.zim"));
#endif
  EXPECT_EQ("3plus2",  path2HumanReadableId("3+2.zim"));
}

TEST(BookTest, getLanguages)
{
  typedef std::vector<std::string> Langs;

  {
    const kiwix::Book book = makeBook(R"(id="abcd" language="fra")");

    EXPECT_EQ(book.getCommaSeparatedLanguages(), "fra");
    EXPECT_EQ(book.getLanguages(), Langs{ "fra" });
  }

  {
    const kiwix::Book book = makeBook(R"(id="abcd" language="eng,ong,ing")");

    EXPECT_EQ(book.getCommaSeparatedLanguages(), "eng,ong,ing");
    EXPECT_EQ(book.getLanguages(), Langs({ "eng", "ong", "ing" }));
  }
}

// OPDS analogue of getLanguages above.
TEST(BookTest, getLanguagesOpds)
{
  typedef std::vector<std::string> Langs;

  {
    const kiwix::Book book = makeBookFromOpds("<id>abcd</id><language>fra</language>");

    EXPECT_EQ(book.getCommaSeparatedLanguages(), "fra");
    EXPECT_EQ(book.getLanguages(), Langs{ "fra" });
  }

  {
    const kiwix::Book book = makeBookFromOpds("<id>abcd</id><language>eng,ong,ing</language>");

    EXPECT_EQ(book.getCommaSeparatedLanguages(), "eng,ong,ing");
    EXPECT_EQ(book.getLanguages(), Langs({ "eng", "ong", "ing" }));
  }
}

TEST(BookTest, updateFromOPDSMultipleThumbnailLinksTest)
{
  // Several rel="...thumbnail" links (e.g. one per size) must all be kept,
  // not just the last one seen.
  const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png"
              href="https://example.com/zara-48.png" />
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png"
              href="https://example.com/zara-96.png" />
    )");

  const auto& illustrations = book.getIllustrations();
  ASSERT_EQ(illustrations.size(), 2U);
  EXPECT_EQ(illustrations[0]->url, "https://example.com/zara-48.png");
  EXPECT_EQ(illustrations[1]->url, "https://example.com/zara-96.png");
}
