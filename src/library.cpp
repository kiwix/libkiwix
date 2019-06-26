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

bool Library::writeToFile(const std::string& path)
{
  auto baseDir = removeLastPathElement(path, true, false);
  LibXMLDumper dumper(this);
  dumper.setBaseDir(baseDir);
  return writeTextFile(path, dumper.dumpLibXMLContent(getBooksIds()));
}

bool Library::writeBookmarksToFile(const std::string& path)
{
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

  return filter(Filter().query(search));
}


std::vector<std::string> Library::filter(const Filter& filter)
{
  std::vector<std::string> bookIds;
  for(auto& pair:m_books) {
    auto book = pair.second;
    if(filter.accept(book)) {
      bookIds.push_back(pair.first);
    }
  }
  return bookIds;
}

template<supportedListSortBy SORT>
struct KEY_TYPE {
  typedef std::string TYPE;
};

template<>
struct KEY_TYPE<SIZE> {
  typedef size_t TYPE;
};

template<supportedListSortBy sort>
class Comparator {
  private:
    Library* lib;
    bool     ascending;

    inline typename KEY_TYPE<sort>::TYPE get_key(const std::string& id);

  public:
    Comparator(Library* lib, bool ascending) : lib(lib), ascending(ascending) {}
    inline bool operator() (const std::string& id1, const std::string& id2) {
      if (ascending) {
        return get_key(id1) < get_key(id2);
      } else {
        return get_key(id2) < get_key(id1);
      }
    }
};

template<>
std::string Comparator<TITLE>::get_key(const std::string& id)
{
  return lib->getBookById(id).getTitle();
}

template<>
size_t Comparator<SIZE>::get_key(const std::string& id)
{
  return lib->getBookById(id).getSize();
}

template<>
std::string Comparator<DATE>::get_key(const std::string& id)
{
  return lib->getBookById(id).getDate();
}

template<>
std::string Comparator<CREATOR>::get_key(const std::string& id)
{
  return lib->getBookById(id).getCreator();
}

template<>
std::string Comparator<PUBLISHER>::get_key(const std::string& id)
{
  return lib->getBookById(id).getPublisher();
}

void Library::sort(std::vector<std::string>& bookIds, supportedListSortBy sort, bool ascending)
{
  switch(sort) {
    case TITLE:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<TITLE>(this, ascending));
      break;
    case SIZE:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<SIZE>(this, ascending));
      break;
    case DATE:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<DATE>(this, ascending));
      break;
    case CREATOR:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<CREATOR>(this, ascending));
      break;
    case PUBLISHER:
      std::sort(bookIds.begin(), bookIds.end(), Comparator<PUBLISHER>(this, ascending));
      break;
    default:
      break;
  }
}


std::vector<std::string> Library::listBooksIds(
    int mode,
    supportedListSortBy sortBy,
    const std::string& search,
    const std::string& language,
    const std::string& creator,
    const std::string& publisher,
    const std::vector<std::string>& tags,
    size_t maxSize) {

  Filter _filter;
  if (mode & LOCAL)
    _filter.local(true);
  if (mode & NOLOCAL)
    _filter.local(false);
  if (mode & VALID)
    _filter.valid(true);
  if (mode & NOVALID)
    _filter.valid(false);
  if (mode & REMOTE)
    _filter.remote(true);
  if (mode & NOREMOTE)
    _filter.remote(false);
  if (!tags.empty())
    _filter.acceptTags(tags);
  if (maxSize != 0)
    _filter.maxSize(maxSize);
  if (!language.empty())
    _filter.lang(language);
  if (!publisher.empty())
    _filter.publisher(publisher);
  if (!creator.empty())
    _filter.creator(creator);
  if (!search.empty())
    _filter.query(search);

  auto bookIds = filter(_filter);

  sort(bookIds, sortBy, true);
  return bookIds;
}

Filter::Filter()
  : activeFilters(0),
    _maxSize(0)
{};

#define FLAG(x) (1 << x)
enum filterTypes {
  NONE = 0,
  _LOCAL = FLAG(0),
  _REMOTE = FLAG(1),
  _NOLOCAL = FLAG(2),
  _NOREMOTE = FLAG(3),
  _VALID = FLAG(4),
  _NOVALID = FLAG(5),
  ACCEPTTAGS = FLAG(6),
  REJECTTAGS = FLAG(7),
  LANG = FLAG(8),
  _PUBLISHER = FLAG(9),
  _CREATOR = FLAG(10),
  MAXSIZE = FLAG(11),
  QUERY = FLAG(12),
};

Filter& Filter::local(bool accept)
{
  if (accept) {
    activeFilters |= _LOCAL;
    activeFilters &= ~_NOLOCAL;
  } else {
    activeFilters |= _NOLOCAL;
    activeFilters &= ~_LOCAL;
  }
  return *this;
}

Filter& Filter::remote(bool accept)
{
  if (accept) {
    activeFilters |= _REMOTE;
    activeFilters &= ~_NOREMOTE;
  } else {
    activeFilters |= _NOREMOTE;
    activeFilters &= ~_REMOTE;
  }
  return *this;
}

Filter& Filter::valid(bool accept)
{
  if (accept) {
    activeFilters |= _VALID;
    activeFilters &= ~_NOVALID;
  } else {
    activeFilters |= _NOVALID;
    activeFilters &= ~_VALID;
  }
  return *this;
}

Filter& Filter::acceptTags(std::vector<std::string> tags)
{
  _acceptTags = tags;
  activeFilters |= ACCEPTTAGS;
  return *this;
}

Filter& Filter::rejectTags(std::vector<std::string> tags)
{
  _rejectTags = tags;
  activeFilters |= REJECTTAGS;
  return *this;
}

Filter& Filter::lang(std::string lang)
{
  _lang = lang;
  activeFilters |= LANG;
  return *this;
}

Filter& Filter::publisher(std::string publisher)
{
  _publisher = publisher;
  activeFilters |= _PUBLISHER;
  return *this;
}

Filter& Filter::creator(std::string creator)
{
  _creator = creator;
  activeFilters |= _CREATOR;
  return *this;
}

Filter& Filter::maxSize(size_t maxSize)
{
  _maxSize = maxSize;
  activeFilters |= MAXSIZE;
  return *this;
}

Filter& Filter::query(std::string query)
{
  _query = query;
  activeFilters |= QUERY;
  return *this;
}

#define ACTIVE(X) (activeFilters & (X))
bool Filter::accept(const Book& book) const
{
  auto local = !book.getPath().empty();
  if (ACTIVE(_LOCAL) && !local)
    return false;
  if (ACTIVE(_NOLOCAL) && local)
    return false;
  auto valid = book.isPathValid();
  if (ACTIVE(_VALID) && !valid)
    return false;
  if (ACTIVE(_NOVALID) && valid)
    return false;
  auto remote = !book.getUrl().empty();
  if (ACTIVE(_REMOTE) && !remote)
    return false;
  if (ACTIVE(_NOREMOTE) && remote)
    return false;
  if (ACTIVE(ACCEPTTAGS)) {
    if (!_acceptTags.empty()) {
      auto vBookTags = split(book.getTags(), ";");
      std::set<std::string> sBookTags(vBookTags.begin(), vBookTags.end());
      for (auto& t: _acceptTags) {
        if (sBookTags.find(t) == sBookTags.end()) {
          return false;
        }
      }
    }
  }
  if (ACTIVE(REJECTTAGS)) {
    if (!_rejectTags.empty()) {
      auto vBookTags = split(book.getTags(), ";");
      std::set<std::string> sBookTags(vBookTags.begin(), vBookTags.end());
      for (auto& t: _rejectTags) {
        if (sBookTags.find(t) != sBookTags.end()) {
          return false;
        }
      }
    }
  }
  if (ACTIVE(MAXSIZE) && book.getSize() > _maxSize)
    return false;

  if (ACTIVE(LANG) && book.getLanguage() != _lang)
    return false;

  if (ACTIVE(_PUBLISHER) && book.getPublisher() != _publisher)
    return false;

  if (ACTIVE(_CREATOR) && book.getCreator() != _creator)
    return false;

  if ( ACTIVE(QUERY)
    && !(matchRegex(book.getTitle(), "\\Q" + _query + "\\E")
        || matchRegex(book.getDescription(), "\\Q" + _query + "\\E")))
    return false;

  return true;

}

}
