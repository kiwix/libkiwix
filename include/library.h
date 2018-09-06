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

namespace pugi {
class xml_node;
}

namespace kiwix
{
enum supportedIndexType { UNKNOWN, XAPIAN };

class OPDSDumper;
class Reader;

/**
 * A class to store information about a book (a zim file)
 */
class Book
{
 public:
  Book();
  ~Book();

  bool update(const Book& other);
  void update(const Reader& reader);
  void updateFromXml(const pugi::xml_node& node, const std::string& baseDir);
  void updateFromOpds(const pugi::xml_node& node);
  static bool sortByTitle(const Book& a, const Book& b);
  static bool sortBySize(const Book& a, const Book& b);
  static bool sortByDate(const Book& a, const Book& b);
  static bool sortByCreator(const Book& a, const Book& b);
  static bool sortByPublisher(const Book& a, const Book& b);
  static bool sortByLanguage(const Book& a, const Book& b);
  string getHumanReadableIdFromPath();

  bool readOnly() const { return m_readOnly; }
  const string& id() const { return m_id; }
  const string& path() const { return m_path; }
  const string& indexPath() const { return m_indexPath; }
  const supportedIndexType& indexType() const { return m_indexType; }
  const string& title() const { return m_title; }
  const string& description() const { return m_description; }
  const string& language() const { return m_language; }
  const string& creator() const { return m_creator; }
  const string& publisher() const { return m_publisher; }
  const string& date() const { return m_date; }
  const string& url() const { return m_url; }
  const string& name() const { return m_name; }
  const string& tags() const { return m_tags; }
  const string& origId() const { return m_origId; }
  const uint64_t& articleCount() const { return m_articleCount; }
  const uint64_t& mediaCount() const { return m_mediaCount; }
  const uint64_t& size() const { return m_size; }
  const string& favicon() const { return m_favicon; }
  const string& faviconMimeType() const { return m_faviconMimeType; }

  void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
  void setId(const std::string& id) { m_id = id; }
  void setPath(const std::string& path);
  void setIndexPath(const std::string& indexPath);
  void setIndexType(supportedIndexType indexType) { m_indexType = indexType;}
  void setTitle(const std::string& title) { m_title = title; }
  void setDescription(const std::string& description) { m_description = description; }
  void setLanguage(const std::string& language) { m_language = language; }
  void setCreator(const std::string& creator) { m_creator = creator; }
  void setPublisher(const std::string& publisher) { m_publisher = publisher; }
  void setDate(const std::string& date) { m_date = date; }
  void setUrl(const std::string& url) { m_url = url; }
  void setName(const std::string& name) { m_name = name; }
  void setTags(const std::string& tags) { m_tags = tags; }
  void setOrigId(const std::string& origId) { m_origId = origId; }
  void setArticleCount(uint64_t articleCount) { m_articleCount = articleCount; }
  void setMediaCount(uint64_t mediaCount) { m_mediaCount = mediaCount; }
  void setSize(uint64_t size) { m_size = size; }
  void setFavicon(const std::string& favicon) { m_favicon = favicon; }
  void setFaviconMimeType(const std::string& faviconMimeType) { m_faviconMimeType = faviconMimeType; }

 protected:
  string m_id;
  string m_path;
  string m_indexPath;
  supportedIndexType m_indexType;
  string m_title;
  string m_description;
  string m_language;
  string m_creator;
  string m_publisher;
  string m_date;
  string m_url;
  string m_name;
  string m_tags;
  string m_origId;
  uint64_t m_articleCount;
  uint64_t m_mediaCount;
  bool m_readOnly;
  uint64_t m_size;
  string m_favicon;
  string m_faviconMimeType;
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
