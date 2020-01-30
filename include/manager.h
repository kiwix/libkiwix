/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef KIWIX_MANAGER_H
#define KIWIX_MANAGER_H

#include "book.h"
#include "library.h"
#include "reader.h"

#include <string>
#include <vector>

namespace pugi {
class xml_document;
}

namespace kiwix
{

class LibraryManipulator {
 public:
  virtual ~LibraryManipulator() {}
  virtual bool addBookToLibrary(Book book) = 0;
  virtual void addBookmarkToLibrary(Bookmark bookmark) = 0;
};

class DefaultLibraryManipulator : public LibraryManipulator {
 public:
  DefaultLibraryManipulator(Library* library) :
    library(library) {}
  virtual ~DefaultLibraryManipulator() {}
  bool addBookToLibrary(Book book) {
    return library->addBook(book);
  }
  void addBookmarkToLibrary(Bookmark bookmark) {
    library->addBookmark(bookmark);
  }
 private:
   kiwix::Library* library;
};

/**
 * A tool to manage a `Library`.
 */
class Manager
{
 public:
  Manager(LibraryManipulator* manipulator);
  Manager(Library* library);
  ~Manager();

  /**
   * Read a `library.xml` and add book in the file to the library.
   *
   * @param path The (utf8) path to the `library.xml`.
   * @param readOnly Set if the libray path could be overwritten latter with
   *                 updated content.
   * @return True if file has been properly parsed.
   */
  bool readFile(const std::string& path, bool readOnly = true, bool trustLibrary = true);

  /**
   * Load a library content store in the string.
   *
   * @param xml The content corresponding of the library xml
   * @param readOnly Set if the libray path could be overwritten latter with
   *                 updated content.
   * @param libraryPath The library path (used to resolve relative path)
   * @return True if the content has been properly parsed.
   */
  bool readXml(const std::string& xml,
               const bool readOnly = true,
               const std::string& libraryPath = "",
               bool trustLibrary = true);

  /**
   * Load a library content stored in a OPDS stream.
   *
   * @param content The content of the OPDS stream.
   * @param readOnly Set if the library path could be overwritten later with
   *                 updated content.
   * @param libraryPath The library path (used to resolve relative path)
   * @return True if the content has been properly parsed.
   */
  bool readOpds(const std::string& content, const std::string& urlHost);


  /**
   * Load a bookmark file.
   *
   * @param path The path of the file to read.
   * @return True if the content has been properly parsed.
   */
  bool readBookmarkFile(const std::string& path);

  /**
   * Add a book to the library.
   *
   * @param pathToOpen The path to the zim file to add.
   * @param pathToSave The path to store in the library in place of pathToOpen.
   * @param url        The url of the book to store in the library.
   * @param checMetaData Tell if we check metadata before adding book to the
   *                     library.
   * @return The id of the book if the book has been added to the library.
   *         Else, an empty string.
   */
  std::string addBookFromPathAndGetId(const std::string& pathToOpen,
                                 const std::string& pathToSave = "",
                                 const std::string& url = "",
                                 const bool checkMetaData = false);

  /**
   * Add a book to the library.
   *
   * @param pathToOpen The path to the zim file to add.
   * @param pathToSave The path to store in the library in place of pathToOpen.
   * @param url        The url of the book to store in the library.
   * @param checMetaData Tell if we check metadata before adding book to the
   *                     library.
   * @return True if the book has been added to the library.
   */

  bool addBookFromPath(const std::string& pathToOpen,
                       const std::string& pathToSave = "",
                       const std::string& url = "",
                       const bool checkMetaData = false);

  std::string writableLibraryPath;

  bool m_hasSearchResult = false;
  uint64_t m_totalBooks = 0;
  uint64_t m_startIndex = 0;
  uint64_t m_itemsPerPage = 0;

 protected:
  kiwix::LibraryManipulator* manipulator;
  bool mustDeleteManipulator;

  bool readBookFromPath(const std::string& path, Book* book);
  bool parseXmlDom(const pugi::xml_document& doc,
                   bool readOnly,
                   const std::string& libraryPath,
                   bool trustLibrary);
  bool parseOpdsDom(const pugi::xml_document& doc,
                    const std::string& urlHost);

};
}

#endif
