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

#include "library.h"
#include "reader.h"

#include <pugixml.hpp>

namespace kiwix
{
/* Constructor */
Book::Book() : m_readOnly(false)
{
}
/* Destructor */
Book::~Book()
{
}
/* Sort functions */
bool Book::sortByTitle(const kiwix::Book& a, const kiwix::Book& b)
{
  return a.m_title < b.m_title;
}

bool Book::sortByDate(const kiwix::Book& a, const kiwix::Book& b)
{
  return a.m_date < b.m_date;
}

bool Book::sortBySize(const kiwix::Book& a, const kiwix::Book& b)
{
  return a.m_size < b.m_size;
}

bool Book::sortByPublisher(const kiwix::Book& a, const kiwix::Book& b)
{
  return a.m_publisher < b.m_publisher;
}

bool Book::sortByCreator(const kiwix::Book& a, const kiwix::Book& b)
{
  return a.m_creator < b.m_creator;
}

bool Book::sortByLanguage(const kiwix::Book& a, const kiwix::Book& b)
{
  return a.m_language < b.m_language;
}

bool Book::update(const kiwix::Book& other)
{
  if (m_readOnly)
    return false;

  m_readOnly = other.m_readOnly;

  if (m_path.empty()) {
    m_path = other.m_path;
  }

  if (m_url.empty()) {
    m_url = other.m_url;
  }

  if (m_tags.empty()) {
    m_tags = other.m_tags;
  }

  if (m_name.empty()) {
    m_name = other.m_name;
  }

  if (m_indexPath.empty()) {
    m_indexPath = other.m_indexPath;
    m_indexType = other.m_indexType;
  }

  if (m_faviconMimeType.empty()) {
    m_favicon = other.m_favicon;
    m_faviconMimeType = other.m_faviconMimeType;
  }
  return true;
}

void Book::update(const kiwix::Reader& reader)
{
  m_path = reader.getZimFilePath();
  m_id = reader.getId();
  m_description = reader.getDescription();
  m_language = reader.getLanguage();
  m_date = reader.getDate();
  m_creator = reader.getCreator();
  m_publisher = reader.getPublisher();
  m_title = reader.getTitle();
  m_name = reader.getName();
  m_tags = reader.getTags();
  m_origId = reader.getOrigId();
  m_articleCount = reader.getArticleCount();
  m_mediaCount = reader.getMediaCount();
  m_size = reader.getFileSize();

  reader.getFavicon(m_favicon, m_faviconMimeType);
}

std::string Book::getHumanReadableIdFromPath()
{
  std::string id = m_path;
  if (!id.empty()) {
    kiwix::removeAccents(id);

#ifdef _WIN32
    id = replaceRegex(id, "", "^.*\\\\");
#else
    id = replaceRegex(id, "", "^.*/");
#endif

    id = replaceRegex(id, "", "\\.zim[a-z]*$");
    id = replaceRegex(id, "_", " ");
    id = replaceRegex(id, "plus", "\\+");
  }
  return id;
}

void Book::setPath(const std::string& path)
{
 m_path = isRelativePath(path)
   ? computeAbsolutePath(getCurrentDirectory(), path)
   : path;
}

void Book::setIndexPath(const std::string& indexPath)
{
  m_indexPath = isRelativePath(indexPath)
    ? computeAbsolutePath(getCurrentDirectory(), indexPath)
    : indexPath;
}

/* Constructor */
Library::Library() : version(KIWIX_LIBRARY_VERSION)
{
}
/* Destructor */
Library::~Library()
{
}


bool Library::addBook(const Book& book)
{
  /* Try to find it */
  std::vector<kiwix::Book>::iterator itr;
  try {
    auto& oldbook = books.at(book.id());
    oldbook.update(book);
    return false;
  } catch (std::out_of_range&) {
    books[book.id()] = book;
    return true;
  }
}


bool Library::removeBookById(const std::string& id)
{
  return books.erase(id) == 1;
}

Book& Library::getBookById(const std::string& id)
{
  return books.at(id);
}

unsigned int Library::getBookCount(const bool localBooks,
                                   const bool remoteBooks)
{
  unsigned int result = 0;
  for (auto& pair: books) {
    auto& book = pair.second;
    if ((!book.path().empty() && localBooks)
        || (book.path().empty() && remoteBooks)) {
      result++;
    }
  }
  return result;
}

Library Library::filter(const std::string& search) {
  Library library;

  if (search.empty()) {
    return library;
  }

  for(auto& pair:books) {
     auto& book = pair.second;
     if (matchRegex(book.title(), "\\Q" + search + "\\E")
         || matchRegex(book.description(), "\\Q" + search + "\\E")) {
       library.addBook(book);
     }
  }

  return library;
}

bool Library::writeToFile(const std::string& path) {
  pugi::xml_document doc;

  /* Add the library node */
  pugi::xml_node libraryNode = doc.append_child("library");

  if (!version.empty())
    libraryNode.append_attribute("version") = version.c_str();

  /* Add each book */
  for (auto& pair: books) {
    auto& book = pair.second;
    if (!book.readOnly()) {
      pugi::xml_node bookNode = libraryNode.append_child("book");
      bookNode.append_attribute("id") = book.id().c_str();

      if (!book.path().empty()) {
        bookNode.append_attribute("path") = computeRelativePath(
            removeLastPathElement(path, true, false), book.path()).c_str();
      }

      if (!book.indexPath().empty()) {
        bookNode.append_attribute("indexPath") = computeRelativePath(
            removeLastPathElement(path, true, false), book.indexPath()).c_str();
        bookNode.append_attribute("indexType") = "xapian";
      }

      if (book.origId().empty()) {
        if (!book.title().empty())
          bookNode.append_attribute("title") = book.title().c_str();

        if (!book.name().empty())
          bookNode.append_attribute("name") = book.name().c_str();

        if (!book.tags().empty())
          bookNode.append_attribute("tags") = book.tags().c_str();

        if (!book.description().empty())
          bookNode.append_attribute("description") = book.description().c_str();

        if (!book.language().empty())
          bookNode.append_attribute("language") = book.language().c_str();

        if (!book.creator().empty())
          bookNode.append_attribute("creator") = book.creator().c_str();

        if (!book.publisher().empty())
          bookNode.append_attribute("publisher") = book.publisher().c_str();

        if (!book.favicon().empty())
          bookNode.append_attribute("favicon") = base64_encode(book.favicon()).c_str();

        if (!book.faviconMimeType().empty())
          bookNode.append_attribute("faviconMimeType")
              = book.faviconMimeType().c_str();
      } else {
        bookNode.append_attribute("origId") = book.origId().c_str();
      }

      if (!book.date().empty()) {
        bookNode.append_attribute("date") = book.date().c_str();
      }

      if (!book.url().empty()) {
        bookNode.append_attribute("url") = book.url().c_str();
      }

      if (!book.articleCount())
        bookNode.append_attribute("articleCount") = to_string(book.articleCount()).c_str();

      if (!book.mediaCount())
        bookNode.append_attribute("mediaCount") = to_string(book.mediaCount()).c_str();

      if (!book.size()) {
        bookNode.append_attribute("size") = to_string(book.size()).c_str();
      }
    }
  }

  /* saving file */
  return doc.save_file(path.c_str());
}

std::vector<std::string> Library::getBooksLanguages()
{
  std::vector<std::string> booksLanguages;
  std::map<std::string, bool> booksLanguagesMap;

  for (auto& pair: books) {
    auto& book = pair.second;
    auto& language = book.language();
    if (booksLanguagesMap.find(language) == booksLanguagesMap.end()) {
      if (book.origId().empty()) {
        booksLanguagesMap[language] = true;
        booksLanguages.push_back(language);
      }
    }
  }

  return booksLanguages;
}

std::vector<std::string> Library::getBooksCreators()
{
  std::vector<std::string> booksCreators;
  std::map<std::string, bool> booksCreatorsMap;

  for (auto& pair: books) {
    auto& book = pair.second;
    auto& creator = book.creator();
    if (booksCreatorsMap.find(creator) == booksCreatorsMap.end()) {
      if (book.origId().empty()) {
        booksCreatorsMap[creator] = true;
        booksCreators.push_back(creator);
      }
    }
  }

  return booksCreators;
}

std::vector<std::string> Library::getBooksIds()
{
  std::vector<std::string> booksIds;

  for (auto& pair: books) {
    booksIds.push_back(pair.first);
  }

  return booksIds;
}

std::vector<std::string> Library::getBooksPublishers()
{
  std::vector<std::string> booksPublishers;
  std::map<std::string, bool> booksPublishersMap;

  for (auto& pair:books) {
    auto& book = pair.second;
    auto& publisher = book.publisher();
    if (booksPublishersMap.find(publisher) == booksPublishersMap.end()) {
      if (book.origId().empty()) {
        booksPublishersMap[publisher] = true;
        booksPublishers.push_back(publisher);
      }
    }
  }

  return booksPublishers;
}



}
