#include "gtest/gtest.h"
#include "../include/book.h"
#include "testing_tools.h"
#include "../src/tools/otherTools.h"
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

TEST(BookTest, updateFromOPDSTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <title>Catch an infection in 24 hours</title>
        <summary>Complete guide to contagious diseases</summary>
        <name>who_contagious_diseases_en</name>
        <kiwix:popularity>46</kiwix:popularity>
        <tags>unittest;_category:medicine;_pictures:yes</tags>
        <category>medicine</category>
        <articleCount>123456</articleCount>
        <mediaCount>234567</mediaCount>
        <kiwix:zimdigests>
          <kiwix:zimdigest type="sha256">ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad</kiwix:zimdigest>
          <kiwix:zimdigest type="md5">8054ec08991a30dafea652998874ead9</kiwix:zimdigest>
        </kiwix:zimdigests>
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

    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "https://who.org/zara.zim");
    EXPECT_EQ(book.getTitle(), "Catch an infection in 24 hours");
    EXPECT_EQ(book.getDescription(), "Complete guide to contagious diseases");
    EXPECT_EQ(book.getTags(), "unittest;_category:medicine;_pictures:yes");
    EXPECT_EQ(book.getName(), "who_contagious_diseases_en");
    EXPECT_TRUE(book.hasPopularity());
    EXPECT_EQ(book.getPopularity(), 46U);
    EXPECT_EQ(book.getCategory(), "medicine");
    EXPECT_EQ(book.getArticleCount(), 123456U);
    EXPECT_EQ(book.getMediaCount(), 234567U);
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "8054ec08991a30dafea652998874ead9");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA1), "");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA256), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    // The OPDS acquisition link's "length" is taken as-is, in bytes.
    EXPECT_EQ(book.getSize(), 345678U);

    // This thumbnail link carries a URL to fetch the image from later (see
    // the updateFromOPDSDataUri* tests for embedded data), so there's no
    // data to check here - just that the URL (prefixed with urlHost) and
    // mime type made it through.
    auto defaultIllustration = book.getIllustration(48);
    EXPECT_EQ(defaultIllustration->getMimeType(), "text/plain");
    EXPECT_EQ(defaultIllustration->getUrl(), "http://who.org/zara.fav");
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
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "");
}

TEST(BookTest, updateFromOPDSMissingPropertiesTest)
{
    // An entry carrying nothing but the mandatory <id>
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
      </entry>
    )");

    kiwix::Book book;
    book.updateFromOpds(opds.child("entry"), "http://who.org", DATA_ABS_PATH);

    // Properties missing from the OPDS entry must be left empty rather than
    // initialized to some fictitious values
    EXPECT_EQ(book.getId(), "zara");
    EXPECT_EQ(book.getPath(), "");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "");
    EXPECT_EQ(book.getTitle(), "");
    EXPECT_EQ(book.getDescription(), "");
    EXPECT_EQ(book.getCommaSeparatedLanguages(), "");
    EXPECT_EQ(book.getCreator(), "");
    EXPECT_EQ(book.getPublisher(), "");
    EXPECT_EQ(book.getDate(), "");
    EXPECT_EQ(book.getName(), "");
    EXPECT_EQ(book.getFlavour(), "");
    EXPECT_EQ(book.getTags(), "");
    EXPECT_EQ(book.getCategory(), "");
    EXPECT_EQ(book.getArticleCount(), 0U);
    EXPECT_EQ(book.getMediaCount(), 0U);
    EXPECT_EQ(book.getSize(), 0U);
    EXPECT_TRUE(book.getIllustrations().empty());
    EXPECT_FALSE(book.hasPopularity());
    EXPECT_EQ(book.getPopularity(), 0U);
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA1), "");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA256), "");
}

TEST(BookTest, updateFromOPDSPopularityTest)
{
    const auto parse = [](kiwix::Book& book, const std::string& popularity) {
      const XMLDoc opds(
        "<entry>"
          "<id>urn:uuid:zara</id>"
          "<kiwix:popularity>" + popularity + "</kiwix:popularity>"
        "</entry>"
      );
      book.updateFromOpds(opds.child("entry"), "http://who.org", "");
    };

    // The boundaries of the valid range. In particular, a popularity of 0
    // must not be confused with an unknown popularity.
    for ( const unsigned validValue : {0U, 1U, 100U} ) {
      kiwix::Book book;
      parse(book, std::to_string(validValue));
      EXPECT_TRUE(book.hasPopularity()) << validValue;
      EXPECT_EQ(book.getPopularity(), validValue);
    }

    for ( const char* invalidValue : {"", "abc", "-1", "101", "4.6", "46%"} ) {
      kiwix::Book book;
      parse(book, invalidValue);
      EXPECT_FALSE(book.hasPopularity()) << invalidValue;
      EXPECT_EQ(book.getPopularity(), 0U) << invalidValue;
    }

    // An entry without (valid) popularity resets a previously known one
    kiwix::Book book;
    parse(book, "46");
    EXPECT_TRUE(book.hasPopularity());
    parse(book, "abc");
    EXPECT_FALSE(book.hasPopularity());
    EXPECT_EQ(book.getPopularity(), 0U);
}

TEST(BookTest, updateFromOPDSDigestsTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <kiwix:zimdigests>
          <kiwix:zimdigest type="sha1">a9993e364706816aba3e25717850c26c9cd0d89d</kiwix:zimdigest>
          <kiwix:zimdigest type="sha256">
            ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
          </kiwix:zimdigest>
          <kiwix:zimdigest type="md5">8054ec08991a30dafea652998874ead9</kiwix:zimdigest>
        </kiwix:zimdigests>
      </entry>
    )");

    kiwix::Book book;
    kiwix::testing::CapturedStderr stderror;
    book.updateFromOpds(opds.child("entry"), "http://who.org", "");

    EXPECT_EQ("", std::string(stderror));
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "8054ec08991a30dafea652998874ead9");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA1), "a9993e364706816aba3e25717850c26c9cd0d89d");
    // Surrounding whitespace is stripped from the digest value
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA256), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST(BookTest, updateFromOPDSSkipsInvalidDigestsTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <kiwix:zimdigests>
          <kiwix:zimdigest type="sha512">cf83e1357eefb8bd</kiwix:zimdigest>
          <kiwix:zimdigest type="SHA256">ba7816bf8f01cfea</kiwix:zimdigest>
          <kiwix:zimdigest>0123456789abcdef</kiwix:zimdigest>
          <kiwix:zimdigest type="sha1">   </kiwix:zimdigest>
          <kiwix:zimdigest type="md5">8054ec08991a30dafea652998874ead9</kiwix:zimdigest>
        </kiwix:zimdigests>
      </entry>
    )");

    kiwix::Book book;
    kiwix::testing::CapturedStderr stderror;
    book.updateFromOpds(opds.child("entry"), "http://who.org", "");

    EXPECT_EQ(
      "Book 'zara': ignoring digest 'cf83e1357eefb8bd': Unknown digest type: 'sha512'\n"
      "Book 'zara': ignoring digest 'ba7816bf8f01cfea': Unknown digest type: 'SHA256'\n"
      "Book 'zara': ignoring digest '0123456789abcdef': Unknown digest type: ''\n"
      "Book 'zara': ignoring empty sha1 digest.\n",
      std::string(stderror));

    // The digests after the skipped ones are still read
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "8054ec08991a30dafea652998874ead9");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA1), "");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA256), "");
}

TEST(BookTest, updateFromOPDSDuplicateDigestWarnsTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <kiwix:zimdigests>
          <kiwix:zimdigest type="md5">8054ec08991a30dafea652998874ead9</kiwix:zimdigest>
          <kiwix:zimdigest type="md5">d41d8cd98f00b204e9800998ecf8427e</kiwix:zimdigest>
        </kiwix:zimdigests>
      </entry>
    )");

    kiwix::Book book;
    kiwix::testing::CapturedStderr stderror;
    book.updateFromOpds(opds.child("entry"), "http://who.org", "");

    EXPECT_EQ(
      "Book 'zara': md5 digest '8054ec08991a30dafea652998874ead9' is "
      "overridden by 'd41d8cd98f00b204e9800998ecf8427e'.\n",
      std::string(stderror));
    // The last digest of a given type wins
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "d41d8cd98f00b204e9800998ecf8427e");
}

TEST(BookTest, updateFromOPDSResetsDigestsTest)
{
    const auto parse = [](kiwix::Book& book, const std::string& digests) {
      const XMLDoc opds(
        "<entry>"
          "<id>urn:uuid:zara</id>"
          + digests +
        "</entry>"
      );
      book.updateFromOpds(opds.child("entry"), "http://who.org", "");
    };

    const char* const withoutDigests[] = {
      // no digests at all
      "",
      // an empty digest list
      "<kiwix:zimdigests/>",
      // a digest outside of <kiwix:zimdigests> isn't recognized
      "<kiwix:zimdigest type=\"sha1\">a9993e364706816aba3e25717850c26c9cd0d89d</kiwix:zimdigest>",
    };

    for ( const char* digests : withoutDigests ) {
      kiwix::Book book;
      parse(book, "<kiwix:zimdigests>"
                    "<kiwix:zimdigest type=\"md5\">8054ec08991a30dafea652998874ead9</kiwix:zimdigest>"
                  "</kiwix:zimdigests>");
      EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "8054ec08991a30dafea652998874ead9");

      parse(book, digests);
      EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "") << digests;
      EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA1), "") << digests;
      EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA256), "") << digests;
    }
}

TEST(BookTest, setPopularityTest)
{
    kiwix::Book book;
    EXPECT_FALSE(book.hasPopularity());
    EXPECT_EQ(book.getPopularity(), 0U);

    book.setPopularity(0);
    EXPECT_TRUE(book.hasPopularity());
    EXPECT_EQ(book.getPopularity(), 0U);

    book.setPopularity(73);
    EXPECT_EQ(book.getPopularity(), 73U);

    book.setPopularity(100);
    EXPECT_TRUE(book.hasPopularity());
    EXPECT_EQ(book.getPopularity(), 100U);

    // Out-of-range values are rejected, leaving the popularity unchanged
    for ( const int invalidValue : {-1, 101} ) {
      EXPECT_THROW(book.setPopularity(invalidValue), std::out_of_range)
        << invalidValue;
      EXPECT_TRUE(book.hasPopularity()) << invalidValue;
      EXPECT_EQ(book.getPopularity(), 100U) << invalidValue;

      kiwix::Book bookWithoutPopularity;
      EXPECT_THROW(bookWithoutPopularity.setPopularity(invalidValue),
                   std::out_of_range) << invalidValue;
      EXPECT_FALSE(bookWithoutPopularity.hasPopularity()) << invalidValue;
    }
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
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "https://who.org/zara.zim");
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

TEST(BookTest, updateFromOPDSMultipleMimeTypeAcquisitionLinksTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="https://who.org/zara.zim"
              length="345678" />
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/metalink4+xml"
              href="https://who.org/zara.zim.meta4"
              length="345678" />
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-bittorrent"
              href="https://who.org/zara.zim.torrent"
              length="345678" />
      </entry>
    )");

    kiwix::Book book;
    book.updateFromOpds(opds.child("entry"), "http://who.org", "");

    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "https://who.org/zara.zim");
    EXPECT_EQ(book.getSize(), 345678U);

    EXPECT_EQ(kiwix::nonEmptyAcquisitionLinksCount(book), 3U);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "https://who.org/zara.zim");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::META4), "https://who.org/zara.zim.meta4");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::BITTORRENT), "https://who.org/zara.zim.torrent");
}

TEST(BookTest, updateFromOPDSSkipsAcquisitionLinkOfUnknownTypeTest)
{
    const XMLDoc opds(R"(
      <entry>
        <id>urn:uuid:zara</id>
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/octet-stream"
              href="https://who.org/zara.bin"
              length="999" />
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="https://who.org/zara.zim"
              length="345678" />
        <link rel="http://opds-spec.org/acquisition/open-access"
              href="zara.zim" />
      </entry>
    )");

    kiwix::Book book;
    kiwix::testing::CapturedStderr stderror;
    book.updateFromOpds(opds.child("entry"), "http://who.org", DATA_ABS_PATH);

    EXPECT_EQ(
      "Book 'zara': ignoring acquisition link 'https://who.org/zara.bin':"
      " Unknown acquisition link type: application/octet-stream\n",
      std::string(stderror));

    // The links after the skipped one are still read...
    EXPECT_EQ(kiwix::nonEmptyAcquisitionLinksCount(book), 1U);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "https://who.org/zara.zim");
    EXPECT_EQ(book.getPath(), ZARA_ABS_PATH);
    // ... and the length of the skipped link is ignored.
    EXPECT_EQ(book.getSize(), 345678U);
}

TEST(BookTest, setUrlStoresUrlPerAcquisitionLinkKind)
{
    kiwix::Book book;
    book.setUrl(kiwix::Book::AcquisitionLinkKind::DIRECT, "http://who.org/zara.zim");
    book.setUrl(kiwix::Book::AcquisitionLinkKind::META4, "http://who.org/zara.zim.meta4");
    book.setUrl(kiwix::Book::AcquisitionLinkKind::BITTORRENT, "http://who.org/zara.zim.torrent");

    EXPECT_EQ(kiwix::nonEmptyAcquisitionLinksCount(book), 3U);
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "http://who.org/zara.zim");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::META4), "http://who.org/zara.zim.meta4");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::BITTORRENT), "http://who.org/zara.zim.torrent");
    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::MAGNET), "");
}

TEST(BookTest, acquisitionLinksHaveOneEmptyEntryPerKind)
{
    const kiwix::Book::AcquisitionLinks urls;
    const size_t kindCount = size_t(kiwix::Book::AcquisitionLinkKind::COUNT);
    EXPECT_EQ(urls.size(), kindCount);
    for ( size_t i = 0; i < kindCount; ++i ) {
      EXPECT_EQ(urls[kiwix::Book::AcquisitionLinkKind(i)], "") << i;
    }
    EXPECT_THROW(urls[kiwix::Book::AcquisitionLinkKind::COUNT], std::out_of_range);
}

TEST(BookTest, digestsAreEmptyByDefault)
{
    const kiwix::Book book;
    const auto digests = book.getDigests();
    EXPECT_EQ(digests.size(), size_t(kiwix::Book::DigestType::COUNT));
    for ( size_t i = 0; i < digests.size(); ++i ) {
      EXPECT_EQ(digests[kiwix::Book::DigestType(i)], "") << i;
    }
    EXPECT_THROW(digests[kiwix::Book::DigestType::COUNT], std::out_of_range);
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::MD5), "");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA1), "");
    EXPECT_EQ(book.getDigest(kiwix::Book::DigestType::SHA256), "");
}

TEST(BookTest, setDigestStoresDigestPerType)
{
    typedef kiwix::Book::DigestType DigestType;

    kiwix::Book book;
    book.setDigest(DigestType::SHA256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    book.setDigest(DigestType::MD5, "8054ec08991a30dafea652998874ead9");

    EXPECT_EQ(book.getDigest(DigestType::MD5), "8054ec08991a30dafea652998874ead9");
    EXPECT_EQ(book.getDigest(DigestType::SHA1), "");
    EXPECT_EQ(book.getDigest(DigestType::SHA256), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    // getDigests() is indexed by DigestType
    const auto digests = book.getDigests();
    EXPECT_EQ(digests[DigestType::MD5], "8054ec08991a30dafea652998874ead9");
    EXPECT_EQ(digests[DigestType::SHA1], "");
    EXPECT_EQ(digests[DigestType::SHA256], "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    // Setting a digest of an already set type overwrites it
    book.setDigest(DigestType::MD5, "d41d8cd98f00b204e9800998ecf8427e");
    EXPECT_EQ(book.getDigest(DigestType::MD5), "d41d8cd98f00b204e9800998ecf8427e");

    // Setting an empty value clears the digest of that type only
    book.setDigest(DigestType::SHA256, "");
    EXPECT_EQ(book.getDigest(DigestType::SHA256), "");
    EXPECT_EQ(book.getDigest(DigestType::MD5), "d41d8cd98f00b204e9800998ecf8427e");
}

TEST(BookTest, digestsAreCopied)
{
    typedef kiwix::Book::DigestType DigestType;
    const std::string sha256 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";

    kiwix::Book book;
    book.setId("xyz");
    book.setDigest(DigestType::SHA256, sha256);

    const kiwix::Book copy(book);
    EXPECT_EQ(copy.getDigest(DigestType::SHA256), sha256);

    kiwix::Book assigned;
    assigned = book;
    EXPECT_EQ(assigned.getDigest(DigestType::SHA256), sha256);

    // update() replaces the digests too, including clearing absent ones
    kiwix::Book updated;
    updated.setId("xyz");
    updated.setDigest(DigestType::MD5, "8054ec08991a30dafea652998874ead9");
    EXPECT_TRUE(updated.update(book));
    EXPECT_EQ(updated.getDigest(DigestType::SHA256), sha256);
    EXPECT_EQ(updated.getDigest(DigestType::MD5), "");

    // Copies are independent of each other
    assigned.setDigest(DigestType::SHA256, "");
    EXPECT_EQ(assigned.getDigest(DigestType::SHA256), "");
    EXPECT_EQ(book.getDigest(DigestType::SHA256), sha256);
}

TEST(BookTest, getUrlIgnoresNonZimAcquisitionLinks)
{
    kiwix::Book book;
    book.setUrl(kiwix::Book::AcquisitionLinkKind::META4, "http://who.org/zara.zim.meta4");
    book.setUrl(kiwix::Book::AcquisitionLinkKind::BITTORRENT, "http://who.org/zara.zim.torrent");

    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "");

    book.setUrl(kiwix::Book::AcquisitionLinkKind::DIRECT, "http://who.org/zara.zim");
    book.setUrl(kiwix::Book::AcquisitionLinkKind::BITTORRENT, "http://who.org/zara.zim.torrent");

    EXPECT_EQ(book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), "http://who.org/zara.zim");
}

namespace
{

kiwix::Book makeBookFromOpds(const std::string& entryContent, const std::string& urlHost="", const std::string& baseDir="")
{
    const XMLDoc opds("<entry>" + entryContent + "</entry>");
    kiwix::Book book;
    book.updateFromOpds(opds.child("entry"), urlHost, baseDir);
    return book;
}

} // unnamed namespace

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
    EXPECT_EQ(illustration->getUrl(), "https://example.com/favicon/zara.png");
}

TEST(BookTest, updateFromOPDSThumbnailLinkTypeWithSizeSuffixTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png;width=96;height=256;scale=1"
              href="https://example.com/zara.png" />
    )");

    const auto illustration = book.getIllustrations().at(0);
    EXPECT_EQ(illustration->getMimeType(), "image/png");
    EXPECT_EQ(illustration->getWidth(), 96);
    EXPECT_EQ(illustration->getHeight(), 256);
}

TEST(BookTest, updateFromOPDSThumbnailLinkTypeWithPartialSizeSuffixTest)
{
    const kiwix::Book book = makeBookFromOpds(R"(
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png;width=96"
              href="https://example.com/zara.png" />
    )");

    const auto illustration = book.getIllustrations().at(0);
    EXPECT_EQ(illustration->getMimeType(), "image/png");
    EXPECT_EQ(illustration->getWidth(), 96);
    EXPECT_EQ(illustration->getHeight(), 48);
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
    EXPECT_EQ(illustration->getUrl(), "");
    EXPECT_EQ(illustration->getMimeType(), "image/jpeg");
    EXPECT_EQ(illustration->getWidth(), 96);
    EXPECT_EQ(illustration->getHeight(), 256);
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
    EXPECT_EQ(illustrations[0]->getWidth(), 48);
    EXPECT_EQ(illustrations[0]->getHeight(), 48);
    EXPECT_EQ(illustrations[1]->getData(), "second-thumbnail");
    EXPECT_EQ(illustrations[1]->getWidth(), 96);
    EXPECT_EQ(illustrations[1]->getHeight(), 96);
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
    EXPECT_EQ(illustrations[0]->getUrl(), "https://example.com/favicon.png");
    EXPECT_EQ(illustrations[1]->getUrl(), "");
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
    EXPECT_EQ(illustration->getUrl(), "");
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
    const kiwix::Book book = makeBookFromOpds("<id>abcd</id><category>ted</category>");

    kiwix::Book newBook;
    newBook.setId("abcd");
    EXPECT_EQ(newBook.getCategory(), "");
    newBook.update(book);
    EXPECT_EQ(newBook.getCategory(), "ted");
}

TEST(BookTest, updateTest)
{
    kiwix::Book book = makeBookFromOpds(R"(
        <id>xyz</id>
        <name>skin-of-color-society_en_all</name>
        <tags>youtube;_videos:yes;_ftindex:yes;_ftindex:yes;_pictures:yes;_details:yes</tags>
        <link rel="http://opds-spec.org/acquisition/open-access"
              href="/home/user/Downloads/skin-of-color-society_en_all_2019-11.zim" />
        <link rel="http://opds-spec.org/acquisition/open-access"
              type="application/x-zim"
              href="https://example.com/book-url.zim" />
        <link rel="http://opds-spec.org/image/thumbnail"
              type="image/png;width=48;height=48;scale=1"
              href="data:image/png;base64,Ym9vay1mYXZpY29u" />
    )", "", "/data/zim");

    book.setPathValid(true);

    kiwix::Book newBook;

    EXPECT_FALSE(newBook.update(book));

    newBook.setId("xyz");
    EXPECT_TRUE(newBook.update(book));

    EXPECT_EQ(newBook.getPath(), book.getPath());
    EXPECT_EQ(newBook.isPathValid(), book.isPathValid());
    EXPECT_EQ(newBook.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT), book.getUrl(kiwix::Book::AcquisitionLinkKind::DIRECT));
    EXPECT_EQ(newBook.getTags(), book.getTags());
    EXPECT_EQ(newBook.getCategory(), book.getCategory());
    EXPECT_EQ(newBook.getName(), book.getName());
    auto defaultIllustration = book.getIllustration(48);
    auto newDefaultIllustration = newBook.getIllustration(48);
    EXPECT_EQ(newDefaultIllustration->getData(), defaultIllustration->getData());
    EXPECT_EQ(newDefaultIllustration->getMimeType(), defaultIllustration->getMimeType());
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
    kiwix::Book book;
    book.setPath(path);
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
  EXPECT_EQ(illustrations[0]->getUrl(), "https://example.com/zara-48.png");
  EXPECT_EQ(illustrations[1]->getUrl(), "https://example.com/zara-96.png");
}
