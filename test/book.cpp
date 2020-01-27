#include "gtest/gtest.h"
#include "../include/book.h"

TEST(BookTest, updateTest)
{
    kiwix::Book book;

    book.setReadOnly(false);
    book.setPath("/home/user/Downloads/skin-of-color-society_en_all_2019-11.zim");
    book.setPathValid(true);
    book.setUrl("book-url");
    book.setTags("youtube;_videos:yes;_ftindex:yes;_ftindex:yes;_pictures:yes;_details:yes");
    book.setName("skin-of-color-society_en_all");
    book.setFavicon("book-favicon");
    book.setFaviconMimeType("book-favicon-mimetype");

    kiwix::Book newBook;

    newBook.setReadOnly(true);
    EXPECT_FALSE(newBook.update(book));

    newBook.setReadOnly(false);
    EXPECT_TRUE(newBook.update(book));

    EXPECT_EQ(newBook.readOnly(), book.readOnly());
    EXPECT_EQ(newBook.getPath(), book.getPath());
    EXPECT_EQ(newBook.isPathValid(), book.isPathValid());
    EXPECT_EQ(newBook.getUrl(), book.getUrl());
    EXPECT_EQ(newBook.getTags(), book.getTags());
    EXPECT_EQ(newBook.getName(), book.getName());
    EXPECT_EQ(newBook.getFavicon(), book.getFavicon());
    EXPECT_EQ(newBook.getFaviconMimeType(), book.getFaviconMimeType());
}
