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

#include <time.h>
#include <sstream>
#include <string>

#include <pugixml.hpp>

#include "common/base64.h"
#include "common/pathTools.h"
#include "common/regexTools.h"
#include "library.h"
#include "reader.h"

using namespace std;

namespace kiwix
{
enum supportedListMode { LASTOPEN, REMOTE, LOCAL };
enum supportedListSortBy { TITLE, SIZE, DATE, CREATOR, PUBLISHER };

/**
 * A tool to manage a `Library`.
 *
 * A `Manager` handle a internal `Library`.
 * This `Library` can be retrived with `cloneLibrary` method.
 */
class Manager
{
 public:
  Manager();
  ~Manager();

  /**
   * Read a `library.xml` and add book in the file to the library.
   *
   * @param path The path to the `library.xml`.
   * @param readOnly Set if the libray path could be overwritten latter with
   *                 updated content.
   * @return True if file has been properly parsed.
   */
  bool readFile(const string path, const bool readOnly = true);

  /**
   * Read a `library.xml` and add book in the file to the library.
   *
   * @param nativePath The path of the `library.xml`
   * @param UTF8Path The utf8 version (?) of the path. Also the path where the
   *                 library will be writen i readOnly is False.
   * @param readOnly Set if the libray path could be overwritten latter with
   *                 updated content.
   * @return True if file has been properly parsed.
   */
  bool readFile(const string nativePath,
                const string UTF8Path,
                const bool readOnly = true);

  /**
   * Load a library content store in the string.
   *
   * @param xml The content corresponding of the library xml
   * @param readOnly Set if the libray path could be overwritten latter with
   *                 updated content.
   * @param libraryPath The library path (used to resolve relative path)
   * @return True if the content has been properly parsed.
   */
  bool readXml(const string& xml,
               const bool readOnly = true,
               const string libraryPath = "");

  /**
   * Load a library content stored in a OPDS stream.
   *
   * @param content The content of the OPDS stream.
   * @param readOnly Set if the library path could be overwritten later with
   *                 updated content.
   * @param libraryPath The library path (used to resolve relative path)
   * @return True if the content has been properly parsed.
   */
  bool readOpds(const string& content, const std::string& urlHost);

  /**
   * Remove a book from the library.
   *
   * @param bookIndex the index of the book to remove
   * @return True
   */
  bool removeBookByIndex(const unsigned int bookIndex);

  /**
   * Remove a book from the library.
   *
   * @param id the id of the book to remove.
   * @return True if the book were in the library.
   */
  bool removeBookById(const string id);

  /**
   * Set the path of the external fulltext index associated to a book.
   *
   * @param id The id of the book to set.
   * @param path The path of the external fullext index.
   * @param supportedIndexType The type of the fulltext index.
   * @return True if the book is in the library.
   */
  bool setBookIndex(const string id,
                    const string path,
                    const supportedIndexType type = XAPIAN);

  /**
   * Set the path of the zim file associated to a book.
   *
   * @param id The id of the book to set.
   * @param path The path of the zim file.
   * @return True if the book is in the library.
   */
  bool setBookPath(const string id, const string path);

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
  string addBookFromPathAndGetId(const string pathToOpen,
                                 const string pathToSave = "",
                                 const string url = "",
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

  bool addBookFromPath(const string pathToOpen,
                       const string pathToSave = "",
                       const string url = "",
                       const bool checkMetaData = false);

  /**
   * Get the book corresponding to an id.
   *
   * @param[in] id The id of the book
   * @param[out] book The book corresponding to the id.
   * @return True if the book has been found.
   */
  bool getBookById(const string id, Book& book);

  /**
   * Update the "last open date" of a book
   *
   * @param id the id of the book.
   * @return True if the book is in the library.
   */
  bool updateBookLastOpenDateById(const string id);

  /**
   * Remove (set to empty) paths of all books in the library.
   */
  void removeBookPaths();

  /**
   * List books in the library.
   *
   * The books list will be available in public vector member `bookIdList`.
   *
   * @param mode The mode of listing :
   *             - LASTOPEN sort by last opened book.
   *             - LOCAL list only local file.
   *             - REMOTE list only remote file.
   * @param sortBy Attribute to sort by the book list.
   * @param maxSize Do not list book bigger than maxSize MiB.
   *                Set to 0 to cancel this filter.
   * @param language List only books in this language.
   * @param creator List only books of this creator.
   * @param publisher List only books of this publisher.
   * @param search List only books with search in the title, description or
   *               language.
   * @return True
   */
  bool listBooks(const supportedListMode mode,
                 const supportedListSortBy sortBy,
                 const unsigned int maxSize,
                 const string language,
                 const string creator,
                 const string publisher,
                 const string search);

  string writableLibraryPath;

  vector<std::string> bookIdList;

 protected:
  kiwix::Library library;

  bool readBookFromPath(const string path, Book* book = NULL);
  bool parseXmlDom(const pugi::xml_document& doc,
                   const bool readOnly,
                   const string libraryPath);
  bool parseOpdsDom(const pugi::xml_document& doc,
                    const std::string& urlHost);

 private:
  void checkAndCleanBookPaths(Book& book, const string& libraryPath);
};
}

#endif
