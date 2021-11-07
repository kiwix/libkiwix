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

TEST(BookTest, updateFromXMLTest)
{
    const XMLDoc xml(R"(
      <book id="zara"
            path="./zara.zim"
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
    book.updateFromXml(xml.child("book"), "/data/zim");

    EXPECT_EQ(book.getPath(), "/data/zim/zara.zim");
    EXPECT_EQ(book.getUrl(), "https://who.org/zara.zim");
    EXPECT_EQ(book.getTitle(), "Catch an infection in 24 hours");
    EXPECT_EQ(book.getDescription(), "Complete guide to contagious diseases");
    EXPECT_EQ(book.getTags(), "unittest;_category:medicine;_pictures:yes");
    EXPECT_EQ(book.getName(), "who_contagious_diseases_en");
    EXPECT_EQ(book.getCategory(), "medicine");
    EXPECT_EQ(book.getArticleCount(), 123456U);
    EXPECT_EQ(book.getMediaCount(), 234567U);
    EXPECT_EQ(book.getSize(), 345678U*1024U);
    EXPECT_EQ(book.getFavicon(), "fake-book-favicon-data");
    EXPECT_EQ(book.getFaviconMimeType(), "text/plain");
    EXPECT_EQ(book.getFaviconUrl(), "http://who.org/zara.fav");
}

TEST(BookTest, updateFromXMLCategoryHandlingTest)
{
  {
    const XMLDoc xml(R"(
      <book id="abcd"
            tags="_category:category_defined_via_tags_only"
          >
      </book>
    )");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), "");

    EXPECT_EQ(book.getCategory(), "category_defined_via_tags_only");
  }
  {
    const XMLDoc xml(R"(
      <book id="abcd"
            category="category_defined_via_attribute_only"
          >
      </book>
    )");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), "");

    EXPECT_EQ(book.getCategory(), "category_defined_via_attribute_only");
  }
  {
    const XMLDoc xml(R"(
      <book id="abcd"
            category="category_attribute_overrides_tags"
            tags="_category:tags_override_category_attribute"
          >
      </book>
    )");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), "");

    EXPECT_EQ(book.getCategory(), "category_attribute_overrides_tags");
  }
  {
    const XMLDoc xml(R"(
      <book id="abcd"
            tags="_category:tags_override_category_attribute"
            category="category_attribute_overrides_tags"
          >
      </book>
    )");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), "");

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
    const XMLDoc xml(R"(<book id="abcd" category="ted"></book>)");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), "");

    kiwix::Book newBook;
    newBook.setId("abcd");
    EXPECT_EQ(newBook.getCategory(), "");
    newBook.update(book);
    EXPECT_EQ(newBook.getCategory(), "ted");
}

TEST(BookTest, updateTest)
{
    const XMLDoc xml(R"(
      <book id="xyz"
            path="/home/user/Downloads/skin-of-color-society_en_all_2019-11.zim"
            url="book-url"
            name="skin-of-color-society_en_all"
            tags="youtube;_videos:yes;_ftindex:yes;_ftindex:yes;_pictures:yes;_details:yes"
            favicon="Ym9vay1mYXZpY29u"
            faviconMimeType="book-favicon-mimetype"
          >
      </book>
    )");

    kiwix::Book book;
    book.updateFromXml(xml.child("book"), "/data/zim");

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
    EXPECT_EQ(newBook.getFavicon(), book.getFavicon());
    EXPECT_EQ(newBook.getFaviconMimeType(), book.getFaviconMimeType());
}
