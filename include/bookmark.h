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

namespace pugi {
class xml_node;
}

namespace kiwix
{

/**
 * A class to store information about a bookmark (an article in a book)
 */
class Bookmark
{
 public:
  Bookmark();
  ~Bookmark();

  void updateFromXml(const pugi::xml_node& node);

  const std::string& getBookId() const { return m_bookId; }
  const std::string& getBookTitle()  const { return m_bookTitle; }
  const std::string& getUrl() const { return m_url; }
  const std::string& getTitle() const { return m_title; }
  const std::string& getLanguage() const { return m_language; }
  const std::string& getDate() const { return m_date; }

  void setBookId(const std::string& bookId) { m_bookId = bookId; }
  void setBookTitle(const std::string& bookTitle) { m_bookTitle = bookTitle; }
  void setUrl(const std::string& url) { m_url = url; }
  void setTitle(const std::string& title) { m_title = title; }
  void setLanguage(const std::string& language) { m_language = language; }
  void setDate(const std::string& date) { m_date = date; }

 protected:
  std::string m_bookId;
  std::string m_bookTitle;
  std::string m_url;
  std::string m_title;
  std::string m_language;
  std::string m_date;
};

}

#endif
