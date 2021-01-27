#include "gtest/gtest.h"
#include "../include/manager.h"
#include "../include/library.h"
#include "../include/book.h"
#include <iostream>
#include <fstream>

TEST(ManagerTest, addBookFromPathAndGetIdTest)
{
    kiwix::Library lib;
    kiwix::Manager manager = kiwix::Manager(&lib);

    auto bookId = manager.addBookFromPathAndGetId("./test/example.zim");
    ASSERT_NE(bookId, "");
    kiwix::Book book = lib.getBookById(bookId);
    EXPECT_EQ(book.getPath(), computeAbsolutePath("", "./test/example.zim"));
    
    const std::string pathToSave = "./pathToSave";
    const std::string url = "url";
    bookId = manager.addBookFromPathAndGetId("./test/example.zim", pathToSave, url, true);
    book = lib.getBookById(bookId);
    auto savedPath = computeAbsolutePath(removeLastPathElement(manager.writableLibraryPath), pathToSave);
    EXPECT_EQ(book.getPath(), savedPath);
    EXPECT_EQ(book.getUrl(), url);
}
