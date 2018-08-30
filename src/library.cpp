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

#include <pugixml.hpp>

namespace kiwix
{
/* Constructor */
Book::Book() : readOnly(false)
{
}
/* Destructor */
Book::~Book()
{
}
/* Sort functions */
bool Book::sortByLastOpen(const kiwix::Book& a, const kiwix::Book& b)
{
  return atoi(a.last.c_str()) > atoi(b.last.c_str());
}

bool Book::sortByTitle(const kiwix::Book& a, const kiwix::Book& b)
{
  return strcmp(a.title.c_str(), b.title.c_str()) < 0;
}

bool Book::sortByDate(const kiwix::Book& a, const kiwix::Book& b)
{
  return strcmp(a.date.c_str(), b.date.c_str()) > 0;
}

bool Book::sortBySize(const kiwix::Book& a, const kiwix::Book& b)
{
  return atoi(a.size.c_str()) < atoi(b.size.c_str());
}

bool Book::sortByPublisher(const kiwix::Book& a, const kiwix::Book& b)
{
  return strcmp(a.publisher.c_str(), b.publisher.c_str()) < 0;
}

bool Book::sortByCreator(const kiwix::Book& a, const kiwix::Book& b)
{
  return strcmp(a.creator.c_str(), b.creator.c_str()) < 0;
}

bool Book::sortByLanguage(const kiwix::Book& a, const kiwix::Book& b)
{
  return strcmp(a.language.c_str(), b.language.c_str()) < 0;
}

bool Book::update(const kiwix::Book& other)
{
  if (readOnly)
    return false;

  readOnly = other.readOnly;

  if (path.empty()) {
    path = other.path;
  }

  if (pathAbsolute.empty()) {
    pathAbsolute = other.pathAbsolute;
  }

  if (url.empty()) {
    url = other.url;
  }

  if (tags.empty()) {
    tags = other.tags;
  }

  if (name.empty()) {
    name = other.name;
  }

  if (indexPath.empty()) {
    indexPath = other.indexPath;
    indexType = other.indexType;
  }

  if (indexPathAbsolute.empty()) {
    indexPathAbsolute = other.indexPathAbsolute;
    indexType = other.indexType;
  }

  if (faviconMimeType.empty()) {
    favicon = other.favicon;
    faviconMimeType = other.faviconMimeType;
  }
  return true;
}

std::string Book::getHumanReadableIdFromPath()
{
  std::string id = pathAbsolute;
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
    auto& oldbook = books.at(book.id);
    oldbook.update(book);
    return false;
  } catch (std::out_of_range&) {
    books[book.id] = book;
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
    if ((!book.path.empty() && localBooks)
        || (book.path.empty() && remoteBooks)) {
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
     if (matchRegex(book.title, "\\Q" + search + "\\E")
         || matchRegex(book.description, "\\Q" + search + "\\E")) {
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
    if (!book.readOnly) {
      pugi::xml_node bookNode = libraryNode.append_child("book");
      bookNode.append_attribute("id") = book.id.c_str();

      if (!book.path.empty()) {
        bookNode.append_attribute("path") = book.path.c_str();
      }

      if (!book.last.empty() && book.last != "undefined") {
        bookNode.append_attribute("last") = book.last.c_str();
      }

      if (!book.indexPath.empty())
        bookNode.append_attribute("indexPath") = book.indexPath.c_str();

      if (!book.indexPath.empty() || !book.indexPathAbsolute.empty()) {
        if (book.indexType == XAPIAN) {
          bookNode.append_attribute("indexType") = "xapian";
        }
      }

      if (book.origId.empty()) {
        if (!book.title.empty())
          bookNode.append_attribute("title") = book.title.c_str();

        if (!book.name.empty())
          bookNode.append_attribute("name") = book.name.c_str();

        if (!book.tags.empty())
          bookNode.append_attribute("tags") = book.tags.c_str();

        if (!book.description.empty())
          bookNode.append_attribute("description") = book.description.c_str();

        if (!book.language.empty())
          bookNode.append_attribute("language") = book.language.c_str();

        if (!book.creator.empty())
          bookNode.append_attribute("creator") = book.creator.c_str();

        if (!book.publisher.empty())
          bookNode.append_attribute("publisher") = book.publisher.c_str();

        if (!book.favicon.empty())
          bookNode.append_attribute("favicon") = book.favicon.c_str();

        if (!book.faviconMimeType.empty())
          bookNode.append_attribute("faviconMimeType")
              = book.faviconMimeType.c_str();
      }

      if (!book.date.empty()) {
        bookNode.append_attribute("date") = book.date.c_str();
      }

      if (!book.url.empty()) {
        bookNode.append_attribute("url") = book.url.c_str();
      }

      if (!book.origId.empty())
        bookNode.append_attribute("origId") = book.origId.c_str();

      if (!book.articleCount.empty())
        bookNode.append_attribute("articleCount") = book.articleCount.c_str();

      if (!book.mediaCount.empty())
        bookNode.append_attribute("mediaCount") = book.mediaCount.c_str();

      if (!book.size.empty()) {
        bookNode.append_attribute("size") = book.size.c_str();
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
    if (booksLanguagesMap.find(book.language) == booksLanguagesMap.end()) {
      if (book.origId.empty()) {
        booksLanguagesMap[book.language] = true;
        booksLanguages.push_back(book.language);
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
    if (booksCreatorsMap.find(book.creator) == booksCreatorsMap.end()) {
      if (book.origId.empty()) {
        booksCreatorsMap[book.creator] = true;
        booksCreators.push_back(book.creator);
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
    if (booksPublishersMap.find(book.publisher) == booksPublishersMap.end()) {
      if (book.origId.empty()) {
        booksPublishersMap[book.publisher] = true;
        booksPublishers.push_back(book.publisher);
      }
    }
  }

  return booksPublishers;
}



}
