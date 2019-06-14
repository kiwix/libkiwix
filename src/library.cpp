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
#include "libxml_dumper.h"

#include "tools/base64.h"
#include "tools/regexTools.h"
#include "tools/pathTools.h"

#include <pugixml.hpp>
#include <algorithm>
#include <set>

namespace kiwix
{
/* Constructor */
Library::Library()
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
    auto& oldbook = m_books.at(book.getId());
    oldbook.update(book);
    return false;
  } catch (std::out_of_range&) {
    m_books[book.getId()] = book;
    return true;
  }
}

void Library::addBookmark(const Bookmark& bookmark)
{
  m_bookmarks.push_back(bookmark);
}

bool Library::removeBookmark(const std::string& zimId, const std::string& url)
{
  for(auto it=m_bookmarks.begin(); it!=m_bookmarks.end(); it++) {
    if (it->getBookId() == zimId && it->getUrl() == url) {
      m_bookmarks.erase(it);
      return true;
    }
  }
  return false;
}



bool Library::removeBookById(const std::string& id)
{
  return m_books.erase(id) == 1;
}

Book& Library::getBookById(const std::string& id)
{
  return m_books.at(id);
}

unsigned int Library::getBookCount(const bool localBooks,
                                   const bool remoteBooks)
{
  unsigned int result = 0;
  for (auto& pair: m_books) {
    auto& book = pair.second;
    if ((!book.getPath().empty() && localBooks)
        || (book.getPath().empty() && remoteBooks)) {
      result++;
    }
  }
  return result;
}

bool Library::writeToFile(const std::string& path) {
  auto baseDir = removeLastPathElement(path, true, false);
  LibXMLDumper dumper(this);
  dumper.setBaseDir(baseDir);
  return writeTextFile(path, dumper.dumpLibXMLContent(getBooksIds()));
}

bool Library::writeBookmarksToFile(const std::string& path) {
  LibXMLDumper dumper(this);
  return writeTextFile(path, dumper.dumpLibXMLBookmark());
}

std::vector<std::string> Library::getBooksLanguages()
{
  std::vector<std::string> booksLanguages;
  std::map<std::string, bool> booksLanguagesMap;

  for (auto& pair: m_books) {
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

  for (auto& pair: m_books) {
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

  for (auto& pair:m_books) {
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

  for (auto& pair: m_books) {
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
  for(auto& pair:m_books) {
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
    const bool sortOrderAsc,
    const std::string& search,
    const std::string& language,
    const std::string& creator,
    const std::string& publisher,
    const std::vector<std::string>& tags,
    size_t maxSize) {

  std::vector<std::string> bookIds;
  for(auto& pair:m_books) {
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
    if (!tags.empty()) {
      auto vBookTags = split(book.getTags(), ";");
      std::set<std::string> sBookTags(vBookTags.begin(), vBookTags.end());
      bool ok = true;
      for (auto& t: tags) {
        if (sBookTags.find(t) == sBookTags.end()) {
          // A "filter" tag is not in the book tag.
          // No need to loop for all "filter" tags.
          ok = false;
          break;
        }
      }
      if (! ok ) {
        // Skip the book
        continue;
      }
    }
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
  if (!sortOrderAsc) {
      std::reverse(bookIds.begin(),bookIds.end());
  }
  return bookIds;
}
}
