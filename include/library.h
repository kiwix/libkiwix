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

#ifndef KIWIX_LIBRARY_H
#define KIWIX_LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>

#include "common/regexTools.h"
#include "common/stringTools.h"

#define KIWIX_LIBRARY_VERSION "20110515"

using namespace std;

namespace kiwix
{
enum supportedIndexType { UNKNOWN, XAPIAN };

class OPDSDumper;

/**
 * A class to store information about a book (a zim file)
 */
class Book
{
 public:
  Book();
  ~Book();

  bool update(const Book& other);
  static bool sortByLastOpen(const Book& a, const Book& b);
  static bool sortByTitle(const Book& a, const Book& b);
  static bool sortBySize(const Book& a, const Book& b);
  static bool sortByDate(const Book& a, const Book& b);
  static bool sortByCreator(const Book& a, const Book& b);
  static bool sortByPublisher(const Book& a, const Book& b);
  static bool sortByLanguage(const Book& a, const Book& b);
  string getHumanReadableIdFromPath();

  string id;
  string path;
  string pathAbsolute;
  string last;
  string indexPath;
  string indexPathAbsolute;
  supportedIndexType indexType;
  string title;
  string description;
  string language;
  string creator;
  string publisher;
  string date;
  string url;
  string name;
  string tags;
  string origId;
  string articleCount;
  string mediaCount;
  bool readOnly;
  string size;
  string favicon;
  string faviconMimeType;
};

/**
 * A Library store several books.
 */
class Library
{
  std::map<std::string, kiwix::Book> books;
 public:
  Library();
  ~Library();

  string version;
  /**
   * Add a book to the library.
   *
   * If a book already exist in the library with the same id, update
   * the existing book instead of adding a new one.
   *
   * @param book The book to add.
   * @return True if the book has been added.
   *         False if a book has been updated.
   */
  bool addBook(const Book& book);

  Book& getBookById(const std::string& id);

  /**
   * Remove a book from the library.
   *
   * @param id the id of the book to remove.
   * @return True if the book were in the lirbrary and has been removed.
   */
  bool removeBookById(const std::string& id);

  /**
   * Write the library to a file.
   *
   * @param path the path of the file to write to.
   * @return True if the library has been correctly save.
   */
  bool writeToFile(const std::string& path);

  /**
   * Get the number of book in the library.
   *
   * @param localBooks If we must count local books (books with a path).
   * @param remoteBooks If we must count remote books (books with an url)
   * @return The number of books.
   */
  unsigned int getBookCount(const bool localBooks, const bool remoteBooks);

  /**
   * Filter the library and generate a new one with the keep elements.
   *
   * @param search List only books with search in the title or description.
   * @return A `Library`.
   */
  Library filter(const string& search);

  /**
   * Get all langagues of the books in the library.
   *
   * @return A list of languages.
   */
  std::vector<std::string> getBooksLanguages();

  /**
   * Get all book creators of the books in the library.
   *
   * @return A list of book creators.
   */
  std::vector<std::string> getBooksCreators();

  /**
   * Get all book publishers of the books in the library.
   *
   * @return A list of book publishers.
   */
  std::vector<std::string> getBooksPublishers();

  /**
   * Get all book ids of the books in the library.
   *
   * @return A list of book ids.
   */
  std::vector<std::string> getBooksIds();

  friend class OPDSDumper;
};
}

#endif
