#include "gtest/gtest.h"
#include "../include/book.h"
#include <pugixml.hpp>

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

namespace
{

kiwix::Book makeBook(const std::string& attr, const std::string& baseDir="")
{
    const XMLDoc xml("<book " + attr + "></book>");
    kiwix::Book book;
    book.updateFromXml(xml.child("book"), baseDir);
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
