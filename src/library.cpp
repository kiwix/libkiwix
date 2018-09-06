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
#include "book.h"

#include "common/base64.h"
#include "common/regexTools.h"
#include "common/pathTools.h"

#include <pugixml.hpp>

namespace kiwix
{
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
    auto& oldbook = books.at(book.getId());
    oldbook.update(book);
    return false;
  } catch (std::out_of_range&) {
    books[book.getId()] = book;
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
    if ((!book.getPath().empty() && localBooks)
        || (book.getPath().empty() && remoteBooks)) {
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
     if (matchRegex(book.getTitle(), "\\Q" + search + "\\E")
         || matchRegex(book.getDescription(), "\\Q" + search + "\\E")) {
       library.addBook(book);
     }
  }

  return library;
}

bool Library::writeToFile(const std::string& path) {
  pugi::xml_document doc;
  auto baseDir = removeLastPathElement(path, true, false);

  /* Add the library node */
  pugi::xml_node libraryNode = doc.append_child("library");

  if (!version.empty())
    libraryNode.append_attribute("version") = version.c_str();

  /* Add each book */
  for (auto& pair: books) {
    auto& book = pair.second;
    if (!book.readOnly()) {
      pugi::xml_node bookNode = libraryNode.append_child("book");
      bookNode.append_attribute("id") = book.getId().c_str();

      if (!book.getPath().empty()) {
        bookNode.append_attribute("path") = computeRelativePath(
            baseDir, book.getPath()).c_str();
      }

      if (!book.getIndexPath().empty()) {
        bookNode.append_attribute("indexPath") = computeRelativePath(
            baseDir, book.getIndexPath()).c_str();
        bookNode.append_attribute("indexType") = "xapian";
      }

      if (book.getOrigId().empty()) {
        if (!book.getTitle().empty())
          bookNode.append_attribute("title") = book.getTitle().c_str();

        if (!book.getName().empty())
          bookNode.append_attribute("name") = book.getName().c_str();

        if (!book.getTags().empty())
          bookNode.append_attribute("tags") = book.getTags().c_str();

        if (!book.getDescription().empty())
          bookNode.append_attribute("description") = book.getDescription().c_str();

        if (!book.getLanguage().empty())
          bookNode.append_attribute("language") = book.getLanguage().c_str();

        if (!book.getCreator().empty())
          bookNode.append_attribute("creator") = book.getCreator().c_str();

        if (!book.getPublisher().empty())
          bookNode.append_attribute("publisher") = book.getPublisher().c_str();

        if (!book.getFavicon().empty())
          bookNode.append_attribute("favicon") = base64_encode(book.getFavicon()).c_str();

        if (!book.getFaviconMimeType().empty())
          bookNode.append_attribute("faviconMimeType")
              = book.getFaviconMimeType().c_str();
      } else {
        bookNode.append_attribute("origId") = book.getOrigId().c_str();
      }

      if (!book.getDate().empty()) {
        bookNode.append_attribute("date") = book.getDate().c_str();
      }

      if (!book.getUrl().empty()) {
        bookNode.append_attribute("url") = book.getUrl().c_str();
      }

      if (!book.getArticleCount())
        bookNode.append_attribute("articleCount") = to_string(book.getArticleCount()).c_str();

      if (!book.getMediaCount())
        bookNode.append_attribute("mediaCount") = to_string(book.getMediaCount()).c_str();

      if (!book.getSize()) {
        bookNode.append_attribute("size") = to_string(book.getSize()).c_str();
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
    auto& language = book.getLanguage();
    if (booksLanguagesMap.find(language) == booksLanguagesMap.end()) {
      if (book.getOrigId().empty()) {
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
    auto& creator = book.getCreator();
    if (booksCreatorsMap.find(creator) == booksCreatorsMap.end()) {
      if (book.getOrigId().empty()) {
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
    auto& publisher = book.getPublisher();
    if (booksPublishersMap.find(publisher) == booksPublishersMap.end()) {
      if (book.getOrigId().empty()) {
        booksPublishersMap[publisher] = true;
        booksPublishers.push_back(publisher);
      }
    }
  }

  return booksPublishers;
}



}
