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

#include "bookmark.h"
#include "book.h"

#include <pugixml.hpp>

namespace kiwix
{

class Bookmark::Impl
{
 public:
  std::string m_bookId;
  std::string m_bookTitle;
  std::string m_bookName;
  std::string m_bookFlavour;
  std::string m_url;
  std::string m_title;
  std::string m_language;
  std::string m_date;
};

/* Constructor */
Bookmark::Bookmark() :
  mp_impl(new Impl())
{
}

Bookmark::Bookmark(const Book& book, const std::string& path, const std::string& title):
  mp_impl(new Impl())
{
  mp_impl->m_bookId = book.getId();
  mp_impl->m_bookTitle = book.getTitle();
  mp_impl->m_bookName = book.getName();
  mp_impl->m_bookFlavour = book.getFlavour();
  mp_impl->m_url = path;
  mp_impl->m_title = title;
  mp_impl->m_language = book.getCommaSeparatedLanguages();
  mp_impl->m_date = book.getDate();
}

Bookmark::Bookmark(const Bookmark& other) :
  mp_impl(new Impl(*other.mp_impl))
{
}

Bookmark::Bookmark(Bookmark&& other) noexcept = default;

Bookmark& Bookmark::operator=(const Bookmark& other)
{
  *mp_impl = *other.mp_impl;
  return *this;
}

Bookmark& Bookmark::operator=(Bookmark&& other) noexcept = default;

/* Destructor */
Bookmark::~Bookmark() = default;

void Bookmark::updateFromXml(const pugi::xml_node& node)
{
  auto bookNode = node.child("book");
  mp_impl->m_bookId = bookNode.child("id").child_value();
  mp_impl->m_bookTitle = bookNode.child("title").child_value();
  mp_impl->m_bookName = bookNode.child("name").child_value();
  mp_impl->m_bookFlavour = bookNode.child("flavour").child_value();
  mp_impl->m_language = bookNode.child("language").child_value();
  mp_impl->m_date = bookNode.child("date").child_value();
  mp_impl->m_title = node.child("title").child_value();
  mp_impl->m_url = node.child("url").child_value();
}

const std::string& Bookmark::getBookId() const { return mp_impl->m_bookId; }
const std::string& Bookmark::getBookTitle() const { return mp_impl->m_bookTitle; }
const std::string& Bookmark::getBookName() const { return mp_impl->m_bookName; }
const std::string& Bookmark::getBookFlavour() const { return mp_impl->m_bookFlavour; }
const std::string& Bookmark::getUrl() const { return mp_impl->m_url; }
const std::string& Bookmark::getTitle() const { return mp_impl->m_title; }
const std::string& Bookmark::getLanguage() const { return mp_impl->m_language; }
const std::string& Bookmark::getDate() const { return mp_impl->m_date; }

void Bookmark::setBookId(const std::string& bookId) { mp_impl->m_bookId = bookId; }
void Bookmark::setBookTitle(const std::string& bookTitle) { mp_impl->m_bookTitle = bookTitle; }
void Bookmark::setBookName(const std::string& bookName) { mp_impl->m_bookName = bookName; }
void Bookmark::setBookFlavour(const std::string& bookFlavour) { mp_impl->m_bookFlavour = bookFlavour; }
void Bookmark::setUrl(const std::string& url) { mp_impl->m_url = url; }
void Bookmark::setTitle(const std::string& title) { mp_impl->m_title = title; }
void Bookmark::setLanguage(const std::string& language) { mp_impl->m_language = language; }
void Bookmark::setDate(const std::string& date) { mp_impl->m_date = date; }

}
