/*
 * Copyright 2018 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_BOOKMARK_H
#define KIWIX_BOOKMARK_H

#include <string>
#include <memory>

namespace pugi {
class xml_node;
}

namespace kiwix
{

class Book;
/**
 * A class to store information about a bookmark (an article in a book)
 */
class Bookmark
{
 public:
  /**
   *  Create an empty bookmark.
   *
   * Bookmark must be populated with `set*` methods
   */
  Bookmark();

  /**
   * Create a bookmark given a Book, a path and a title.
   */
  Bookmark(const Book& book, const std::string& path, const std::string& title);

  Bookmark(const Bookmark& other);
  Bookmark(Bookmark&& other) noexcept;
  Bookmark& operator=(const Bookmark& other);
  Bookmark& operator=(Bookmark&& other) noexcept;

  ~Bookmark();

  void updateFromXml(const pugi::xml_node& node);

  const std::string& getBookId() const;
  const std::string& getBookTitle() const;
  const std::string& getBookName() const;
  const std::string& getBookFlavour() const;
  const std::string& getUrl() const;
  const std::string& getTitle() const;
  const std::string& getLanguage() const;
  const std::string& getDate() const;

  void setBookId(const std::string& bookId);
  void setBookTitle(const std::string& bookTitle);
  void setBookName(const std::string& bookName);
  void setBookFlavour(const std::string& bookFlavour);
  void setUrl(const std::string& url);
  void setTitle(const std::string& title);
  void setLanguage(const std::string& language);
  void setDate(const std::string& date);

 private:
  class Impl;
  std::unique_ptr<Impl> mp_impl;
};

}

#endif
