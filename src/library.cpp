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
#include <algorithm>

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

      if (book.getSize()) {
        bookNode.append_attribute("size") = to_string(book.getSize()>>10).c_str();
      }

      if (!book.getDownloadId().empty()) {
        bookNode.append_attribute("downloadId") = book.getDownloadId().c_str();
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

std::vector<std::string> Library::getBooksIds()
{
  std::vector<std::string> bookIds;

  for (auto& pair: books) {
    bookIds.push_back(pair.first);
  }

  return bookIds;
}

std::vector<std::string> Library::filter(const std::string& search)
{
  if (search.empty()) {
    return getBooksIds();
  }

  std::vector<std::string> bookIds;
  for(auto& pair:books) {
     auto& book = pair.second;
     if (matchRegex(book.getTitle(), "\\Q" + search + "\\E")
         || matchRegex(book.getDescription(), "\\Q" + search + "\\E")) {
       bookIds.push_back(pair.first);
     }
  }

  return bookIds;
}

template<supportedListSortBy sort>
struct Comparator {
  Library* lib;
  Comparator(Library* lib) : lib(lib) {}

  bool operator() (const std::string& id1, const std::string& id2) {
    return get_keys(id1) < get_keys(id2);
  }

  std::string get_keys(const std::string& id);
  unsigned int get_keyi(const std::string& id);
};

template<>
std::string Comparator<TITLE>::get_keys(const std::string& id)
{
  return lib->getBookById(id).getTitle();
}

template<>
unsigned int Comparator<SIZE>::get_keyi(const std::string& id)
{
  return lib->getBookById(id).getSize();
}

template<>
bool Comparator<SIZE>::operator() (const std::string& id1, const std::string& id2)
{
  return get_keyi(id1) < get_keyi(id2);
}

template<>
std::string Comparator<DATE>::get_keys(const std::string& id)
{
  return lib->getBookById(id).getDate();
}

template<>
std::string Comparator<CREATOR>::get_keys(const std::string& id)
{
  return lib->getBookById(id).getCreator();
}

template<>
std::string Comparator<PUBLISHER>::get_keys(const std::string& id)
{
  return lib->getBookById(id).getPublisher();
}


std::vector<std::string> Library::listBooksIds(
    int mode,
    supportedListSortBy sortBy,
    const std::string& search,
    const std::string& language,
    const std::string& creator,
    const std::string& publisher,
    size_t maxSize) {

  std::vector<std::string> bookIds;
  for(auto& pair:books) {
    auto& book = pair.second;
    auto local = !book.getPath().empty();
    if (mode & LOCAL && !local)
      continue;
    if (mode & NOLOCAL && local)
      continue;
    auto valid = book.isPathValid();
    if (mode & VALID && !valid)
      continue;
    if (mode & NOVALID && valid)
      continue;
    auto remote = !book.getUrl().empty();
    if (mode & REMOTE && !remote)
      continue;
    if (mode & NOREMOTE && remote)
      continue;
    if (maxSize != 0 && book.getSize() > maxSize)
      continue;
    if (!language.empty() && book.getLanguage() != language)
      continue;
    if (!publisher.empty() && book.getPublisher() != publisher)
      continue;
    if (!creator.empty() && book.getCreator() != creator)
      continue;
    if (!search.empty() && !(matchRegex(book.getTitle(), "\\Q" + search + "\\E")
                          || matchRegex(book.getDescription(), "\\Q" + search + "\\E")))
      continue;
    bookIds.push_back(pair.first);
  }

  switch(sortBy) {
    case TITLE:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<TITLE>(this));
      break;
    case SIZE:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<SIZE>(this));
      break;
    case DATE:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<DATE>(this));
      break;
    case CREATOR:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<CREATOR>(this));
      break;
    case PUBLISHER:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<PUBLISHER>(this));
      break;
    default:
      break;
  }
  return bookIds;
}
}
