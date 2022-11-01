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

#include "tools.h"
#include "tools/base64.h"
#include "tools/regexTools.h"
#include "tools/pathTools.h"
#include "tools/stringTools.h"
#include "tools/otherTools.h"
#include "tools/concurrent_cache.h"
#include "name_mapper.h"

#include <pugixml.hpp>
#include <algorithm>
#include <set>
#include <cmath>
#include <unicode/locid.h>
#include <xapian.h>

namespace kiwix
{

namespace
{

std::string iso639_3ToXapian(const std::string& lang) {
  return icu::Locale(lang.c_str()).getLanguage();
};

std::string normalizeText(const std::string& text)
{
  return removeAccents(text);
}

bool booksReferToTheSameArchive(const Book& book1, const Book& book2)
{
  return book1.isPathValid()
      && book2.isPathValid()
      && book1.getPath() == book2.getPath();
}

template<typename Key, typename Value>
class MultiKeyCache: public ConcurrentCache<std::set<Key>, Value>
{
  public:
    explicit MultiKeyCache(size_t maxEntries)
      : ConcurrentCache<std::set<Key>, Value>(maxEntries)
    {}

    bool drop(const Key& key)
    {
      std::unique_lock<std::mutex> l(this->lock_);
      bool removed = false;
      for(auto& cache_key: this->impl_.keys()) {
        if(cache_key.find(key)!=cache_key.end()) {
          removed |= this->impl_.drop(cache_key);
        }
      }
      return removed;
    }
};

} // unnamed namespace

struct Library::Impl
{
  struct Entry : Book
  {
    Library::Revision lastUpdatedRevision = 0;
  };

  Library::Revision m_revision;
  std::map<std::string, Entry> m_books;
  using ArchiveCache = ConcurrentCache<std::string, std::shared_ptr<zim::Archive>>;
  std::unique_ptr<ArchiveCache> mp_archiveCache;
  using SearcherCache = MultiKeyCache<std::string, std::shared_ptr<ZimSearcher>>;
  std::unique_ptr<SearcherCache> mp_searcherCache;
  std::vector<kiwix::Bookmark> m_bookmarks;
  Xapian::WritableDatabase m_bookDB;

  unsigned int getBookCount(const bool localBooks, const bool remoteBooks) const;

  Impl();
  ~Impl();

  Impl(Impl&& );
  Impl& operator=(Impl&& );
};

Library::Impl::Impl()
  : mp_archiveCache(new ArchiveCache(std::max(getEnvVar<int>("KIWIX_ARCHIVE_CACHE_SIZE", 1), 1))),
    mp_searcherCache(new SearcherCache(std::max(getEnvVar<int>("KIWIX_SEARCHER_CACHE_SIZE", 1), 1))),
    m_bookDB("", Xapian::DB_BACKEND_INMEMORY)
{
}

Library::Impl::~Impl()
{
}

Library::Impl::Impl(Library::Impl&& ) = default;
Library::Impl& Library::Impl::operator=(Library::Impl&& ) = default;

unsigned int
Library::Impl::getBookCount(const bool localBooks, const bool remoteBooks) const
{
  unsigned int result = 0;
  for (auto& pair: m_books) {
    auto& book = pair.second;
    if ((!book.getPath().empty() && localBooks)
        || (!book.getUrl().empty() && remoteBooks)) {
      result++;
    }
  }
  return result;
}

/* Constructor */
Library::Library()
  : mp_impl(new Library::Impl)
{
}

Library::Library(Library&& other)
  : mp_impl(std::move(other.mp_impl))
{
}

Library& Library::operator=(Library&& other)
{
  mp_impl = std::move(other.mp_impl);
  return *this;
}

/* Destructor */
Library::~Library() = default;

bool Library::addBook(const Book& book)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ++mp_impl->m_revision;
  /* Try to find it */
  updateBookDB(book);
  try {
    auto& oldbook = mp_impl->m_books.at(book.getId());
    if ( ! booksReferToTheSameArchive(oldbook, book) ) {
      dropCache(book.getId());
    }
    oldbook.update(book); // XXX: This may have no effect if oldbook is readonly
                          // XXX: Then m_bookDB will become out-of-sync with
                          // XXX: the real contents of the library.
    oldbook.lastUpdatedRevision = mp_impl->m_revision;
    return false;
  } catch (std::out_of_range&) {
    auto& newEntry = mp_impl->m_books[book.getId()];
    static_cast<Book&>(newEntry) = book;
    newEntry.lastUpdatedRevision = mp_impl->m_revision;
    size_t new_cache_size = static_cast<size_t>(std::ceil(mp_impl->getBookCount(true, true)*0.1));
    if (getEnvVar<int>("KIWIX_ARCHIVE_CACHE_SIZE", -1) <= 0) {
      mp_impl->mp_archiveCache->setMaxSize(new_cache_size);
    }
    if (getEnvVar<int>("KIWIX_SEARCHER_CACHE_SIZE", -1) <= 0) {
      mp_impl->mp_searcherCache->setMaxSize(new_cache_size);
    }
    return true;
  }
}

void Library::addBookmark(const Bookmark& bookmark)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  mp_impl->m_bookmarks.push_back(bookmark);
}

bool Library::removeBookmark(const std::string& zimId, const std::string& url)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto it=mp_impl->m_bookmarks.begin(); it!=mp_impl->m_bookmarks.end(); it++) {
    if (it->getBookId() == zimId && it->getUrl() == url) {
      mp_impl->m_bookmarks.erase(it);
      return true;
    }
  }
  return false;
}


void Library::dropCache(const std::string& id)
{
  mp_impl->mp_archiveCache->drop(id);
  mp_impl->mp_searcherCache->drop(id);
}

bool Library::removeBookById(const std::string& id)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  mp_impl->m_bookDB.delete_document("Q" + id);
  dropCache(id);
  // We do not change the cache size here
  // Most of the time, the book is remove in case of library refresh, it is
  // often associated with addBook calls (which will properly set the cache size)
  // Having a too big cache is not a problem here (or it would have been before)
  // (And setMaxSize doesn't actually reduce the cache size, extra cached items
  //  will be removed in put or getOrPut).
  const bool bookWasRemoved = mp_impl->m_books.erase(id) == 1;
  if ( bookWasRemoved ) {
    ++mp_impl->m_revision;
  }
  return bookWasRemoved;
}

Library::Revision Library::getRevision() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return mp_impl->m_revision;
}

uint32_t Library::removeBooksNotUpdatedSince(Revision libraryRevision)
{
  BookIdCollection booksToRemove;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    for ( const auto& entry : mp_impl->m_books) {
      if ( entry.second.lastUpdatedRevision <= libraryRevision ) {
        booksToRemove.push_back(entry.first);
      }
    }
  }

  uint32_t countOfRemovedBooks = 0;
  for ( const auto& id : booksToRemove ) {
    if ( removeBookById(id) )
      ++countOfRemovedBooks;
  }
  return countOfRemovedBooks;
}

const Book& Library::getBookById(const std::string& id) const
{
  // XXX: Doesn't make sense to lock this operation since it cannot
  // XXX: guarantee thread-safety because of its return type
  return mp_impl->m_books.at(id);
}

Book Library::getBookByIdThreadSafe(const std::string& id) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return getBookById(id);
}

const Book& Library::getBookByPath(const std::string& path) const
{
  // XXX: Doesn't make sense to lock this operation since it cannot
  // XXX: guarantee thread-safety because of its return type
  for(auto& it: mp_impl->m_books) {
    auto& book = it.second;
    if (book.getPath() == path)
      return book;
  }
  std::ostringstream ss;
  ss << "No book with path " << path << " in the library." << std::endl;
  throw std::out_of_range(ss.str());
}

std::shared_ptr<zim::Archive> Library::getArchiveById(const std::string& id)
{
  try {
    return mp_impl->mp_archiveCache->getOrPut(id,
    [&](){
      auto book = getBookById(id);
      if (!book.isPathValid()) {
        throw std::invalid_argument("");
      }
      return std::make_shared<zim::Archive>(book.getPath());
    });
  } catch (std::invalid_argument&) {
    return nullptr;
  }
}

std::shared_ptr<ZimSearcher> Library::getSearcherByIds(const BookIdSet& ids)
{
  assert(!ids.empty());
  try {
    return mp_impl->mp_searcherCache->getOrPut(ids,
    [&](){
      std::vector<zim::Archive> archives;
      for(auto& id:ids) {
        auto archive = getArchiveById(id);
        if(!archive) {
          throw std::invalid_argument("");
        }
        archives.push_back(*archive);
      }
      return std::make_shared<ZimSearcher>(zim::Searcher(archives));
    });
  } catch (std::invalid_argument&) {
    return nullptr;
  }
}

unsigned int Library::getBookCount(const bool localBooks,
                                   const bool remoteBooks) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return mp_impl->getBookCount(localBooks, remoteBooks);
}

bool Library::writeToFile(const std::string& path) const
{
  const auto allBookIds = getBooksIds();

  auto baseDir = removeLastPathElement(path);
  LibXMLDumper dumper(this);
  dumper.setBaseDir(baseDir);
  std::string xml;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    xml = dumper.dumpLibXMLContent(allBookIds);
  };
  return writeTextFile(path, xml);
}

bool Library::writeBookmarksToFile(const std::string& path) const
{
  LibXMLDumper dumper(this);
  // NOTE: LibXMLDumper::dumpLibXMLBookmark uses Library in a thread-safe way
  const std::string xml = dumper.dumpLibXMLBookmark();
  return writeTextFile(path, xml);
}

Library::AttributeCounts Library::getBookAttributeCounts(BookStrPropMemFn p) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  AttributeCounts propValueCounts;

  for (const auto& pair: mp_impl->m_books) {
    const auto& book = pair.second;
    if (book.getOrigId().empty()) {
      propValueCounts[(book.*p)()] += 1;
    }
  }
  return propValueCounts;
}

std::vector<std::string> Library::getBookPropValueSet(BookStrPropMemFn p) const
{
  std::vector<std::string> result;
  for ( const auto& kv : getBookAttributeCounts(p) ) {
    result.push_back(kv.first);
  }
  return result;
}

std::vector<std::string> Library::getBooksLanguages() const
{
  return getBookPropValueSet(&Book::getLanguage);
}

Library::AttributeCounts Library::getBooksLanguagesWithCounts() const
{
  return getBookAttributeCounts(&Book::getLanguage);
}

std::vector<std::string> Library::getBooksCategories() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::set<std::string> categories;

  for (const auto& pair: mp_impl->m_books) {
    const auto& book = pair.second;
    const auto& c = book.getCategory();
    if ( !c.empty() ) {
      categories.insert(c);
    }
  }

  return std::vector<std::string>(categories.begin(), categories.end());
}

std::vector<std::string> Library::getBooksCreators() const
{
  return getBookPropValueSet(&Book::getCreator);
}

std::vector<std::string> Library::getBooksPublishers() const
{
  return getBookPropValueSet(&Book::getPublisher);
}

const std::vector<kiwix::Bookmark> Library::getBookmarks(bool onlyValidBookmarks) const
{
  if (!onlyValidBookmarks) {
    return mp_impl->m_bookmarks;
  }
  std::vector<kiwix::Bookmark> validBookmarks;
  auto booksId = getBooksIds();
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto& bookmark:mp_impl->m_bookmarks) {
    if (std::find(booksId.begin(), booksId.end(), bookmark.getBookId()) != booksId.end()) {
      validBookmarks.push_back(bookmark);
    }
  }
  return validBookmarks;
}

Library::BookIdCollection Library::getBooksIds() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  BookIdCollection bookIds;

  for (auto& pair: mp_impl->m_books) {
    bookIds.push_back(pair.first);
  }

  return bookIds;
}


void Library::updateBookDB(const Book& book)
{
  Xapian::Stem stemmer;
  Xapian::TermGenerator indexer;
  const std::string lang = book.getLanguage();
  try {
    stemmer = Xapian::Stem(iso639_3ToXapian(lang));
    indexer.set_stemmer(stemmer);
    indexer.set_stemming_strategy(Xapian::TermGenerator::STEM_SOME);
  } catch (...) {}
  Xapian::Document doc;
  indexer.set_document(doc);

  const std::string title = normalizeText(book.getTitle());
  const std::string desc = normalizeText(book.getDescription());

  // Index title and description without prefixes for general search
  indexer.index_text(title);
  indexer.increase_termpos();
  indexer.index_text(desc);

  // Index all fields for field-based search
  indexer.index_text(title, 1, "S");
  indexer.index_text(desc,  1, "XD");
  indexer.index_text(lang,  1, "L");
  indexer.index_text(normalizeText(book.getCreator()),   1, "A");
  indexer.index_text(normalizeText(book.getPublisher()), 1, "XP");
  indexer.index_text(normalizeText(book.getName()),      1, "XN");
  indexer.index_text(normalizeText(book.getCategory()),  1, "XC");
  const auto bookName = book.getHumanReadableIdFromPath();
  const auto aliasName = HumanReadableNameMapper::removeDateFromBookId(bookName);
  indexer.index_text(normalizeText(aliasName), 1, "XF");

  for ( const auto& tag : split(normalizeText(book.getTags()), ";") ) {
    doc.add_boolean_term("XT" + tag);
    if ( tag[0] != '_' ) {
      indexer.increase_termpos();
      indexer.index_text(tag);
    }
  }

  const std::string idterm = "Q" + book.getId();
  doc.add_boolean_term(idterm);

  doc.set_data(book.getId());

  mp_impl->m_bookDB.replace_document(idterm, doc);
}

namespace
{

bool willSelectEverything(const Xapian::Query& query)
{
  return query.get_type() == Xapian::Query::LEAF_MATCH_ALL;
}


Xapian::Query buildXapianQueryFromFilterQuery(const Filter& filter)
{
  if ( !filter.hasQuery() || filter.getQuery().empty() ) {
    // This is a thread-safe way to construct an equivalent of
    // a Xapian::Query::MatchAll query
    return Xapian::Query(std::string());
  }

  Xapian::QueryParser queryParser;
  queryParser.set_default_op(Xapian::Query::OP_AND);
  queryParser.add_prefix("title", "S");
  queryParser.add_prefix("description", "XD");
  queryParser.add_prefix("name", "XN");
  queryParser.add_prefix("category", "XC");
  queryParser.add_prefix("lang", "L");
  queryParser.add_prefix("publisher", "XP");
  queryParser.add_prefix("creator", "A");
  queryParser.add_prefix("tag", "XT");
  queryParser.add_prefix("filename", "XF");
  const auto partialQueryFlag = filter.queryIsPartial()
                              ? Xapian::QueryParser::FLAG_PARTIAL
                              : 0;
  // Language assumed for the query is not known for sure so stemming
  // is not applied
  //queryParser.set_stemmer(Xapian::Stem(iso639_3ToXapian(???)));
  //queryParser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
  const auto flags = Xapian::QueryParser::FLAG_PHRASE
                   | Xapian::QueryParser::FLAG_BOOLEAN
                   | Xapian::QueryParser::FLAG_BOOLEAN_ANY_CASE
                   | Xapian::QueryParser::FLAG_LOVEHATE
                   | Xapian::QueryParser::FLAG_WILDCARD
                   | partialQueryFlag;
  return queryParser.parse_query(normalizeText(filter.getQuery()), flags);
}

Xapian::Query makePhraseQuery(const std::string& query, const std::string& prefix)
{
  Xapian::QueryParser queryParser;
  queryParser.set_default_op(Xapian::Query::OP_OR);
  queryParser.set_stemming_strategy(Xapian::QueryParser::STEM_NONE);
  const auto flags = 0;
  const auto q = queryParser.parse_query(normalizeText(query), flags, prefix);
  return Xapian::Query(Xapian::Query::OP_PHRASE, q.get_terms_begin(), q.get_terms_end(), q.get_length());
}

Xapian::Query nameQuery(const std::string& name)
{
  return Xapian::Query("XN" + normalizeText(name));
}

Xapian::Query categoryQuery(const std::string& category)
{
  return Xapian::Query("XC" + normalizeText(category));
}

Xapian::Query aliasNamesQuery(const Filter::AliasNames& aliasNames)
{
  Xapian::Query q = Xapian::Query(std::string());
  std::vector<Xapian::Query> queryVec;
  for (const auto& aliasName : aliasNames) {
    queryVec.push_back(makePhraseQuery(aliasName, "XF"));
  }
  Xapian::Query combinedQuery(Xapian::Query::OP_OR, queryVec.begin(), queryVec.end());
  q = Xapian::Query(Xapian::Query::OP_FILTER, q, combinedQuery);
  return q;
}

Xapian::Query langQuery(const std::string& lang)
{
  return Xapian::Query("L" + normalizeText(lang));
}

Xapian::Query publisherQuery(const std::string& publisher)
{
  return makePhraseQuery(publisher, "XP");
}

Xapian::Query creatorQuery(const std::string& creator)
{
  return makePhraseQuery(creator, "A");
}

Xapian::Query tagsQuery(const Filter::Tags& acceptTags, const Filter::Tags& rejectTags)
{
  Xapian::Query q = Xapian::Query(std::string());
  if (!acceptTags.empty()) {
    for ( const auto& tag : acceptTags )
      q &= Xapian::Query("XT" + normalizeText(tag));
  }

  if (!rejectTags.empty()) {
    for ( const auto& tag : rejectTags )
      q = Xapian::Query(Xapian::Query::OP_AND_NOT, q, "XT" + normalizeText(tag));
  }
  return q;
}

Xapian::Query buildXapianQuery(const Filter& filter)
{
  auto q = buildXapianQueryFromFilterQuery(filter);
  if ( filter.hasName() ) {
    q = Xapian::Query(Xapian::Query::OP_AND, q, nameQuery(filter.getName()));
  }
  if ( filter.hasCategory() ) {
    q = Xapian::Query(Xapian::Query::OP_AND, q, categoryQuery(filter.getCategory()));
  }
  if ( filter.hasLang() ) {
    q = Xapian::Query(Xapian::Query::OP_AND, q, langQuery(filter.getLang()));
  }
  if ( filter.hasPublisher() ) {
    q = Xapian::Query(Xapian::Query::OP_AND, q, publisherQuery(filter.getPublisher()));
  }
  if ( filter.hasCreator() ) {
    q = Xapian::Query(Xapian::Query::OP_AND, q, creatorQuery(filter.getCreator()));
  }
  if ( !filter.getAcceptTags().empty() || !filter.getRejectTags().empty() ) {
    const auto tq = tagsQuery(filter.getAcceptTags(), filter.getRejectTags());
    q = Xapian::Query(Xapian::Query::OP_AND, q, tq);;
  }
  if ( !filter.getAliasNames().empty() ) {
    q = Xapian::Query(Xapian::Query::OP_AND, q, aliasNamesQuery(filter.getAliasNames()));
  }
  return q;
}

} // unnamed namespace

Library::BookIdCollection Library::filterViaBookDB(const Filter& filter) const
{
  const auto query = buildXapianQuery(filter);

  if ( willSelectEverything(query) )
    return getBooksIds();

  BookIdCollection bookIds;

  std::lock_guard<std::mutex> lock(m_mutex);
  Xapian::Enquire enquire(mp_impl->m_bookDB);
  enquire.set_query(query);
  const auto results = enquire.get_mset(0, mp_impl->m_books.size());
  for ( auto it = results.begin(); it != results.end(); ++it  ) {
    bookIds.push_back(it.get_document().get_data());
  }

  return bookIds;
}

Library::BookIdCollection Library::filter(const Filter& filter) const
{
  BookIdCollection result;
  const auto preliminaryResult = filterViaBookDB(filter);
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto id : preliminaryResult) {
    if(filter.accept(mp_impl->m_books.at(id))) {
      result.push_back(id);
    }
  }
  return result;
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
    const Library* const lib;
    const bool     ascending;

    inline typename KEY_TYPE<sort>::TYPE get_key(const std::string& id);

  public:
    Comparator(const Library* lib, bool ascending) : lib(lib), ascending(ascending) {}
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

void Library::sort(BookIdCollection& bookIds, supportedListSortBy sort, bool ascending) const
{
  // NOTE: Can reimplement this method in a way that doesn't require locking
  // NOTE: for the entire duration of the sort. Will need to obtain (under a
  // NOTE: lock) the required atributes from the books once, and then the
  // NOTE: sorting will run on a copy of data without locking.
  std::lock_guard<std::mutex> lock(m_mutex);
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
  NAME = FLAG(13),
  CATEGORY = FLAG(14),
  ALIASNAMES = FLAG(15),
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

Filter& Filter::acceptTags(const Tags& tags)
{
  _acceptTags = tags;
  activeFilters |= ACCEPTTAGS;
  return *this;
}

Filter& Filter::rejectTags(const Tags& tags)
{
  _rejectTags = tags;
  activeFilters |= REJECTTAGS;
  return *this;
}

Filter& Filter::category(std::string category)
{
  _category = category;
  activeFilters |= CATEGORY;
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

Filter& Filter::query(std::string query, bool partial)
{
  _query = query;
  _queryIsPartial = partial;
  activeFilters |= QUERY;
  return *this;
}

Filter& Filter::name(std::string name)
{
  _name = name;
  activeFilters |= NAME;
  return *this;
}

Filter& Filter::aliasNames(const AliasNames& aliasNames)
{
  _aliasNames = aliasNames;
  activeFilters |= ALIASNAMES;  
  return *this;
}

#define ACTIVE(X) (activeFilters & (X))
#define FILTER(TAG, TEST) if (ACTIVE(TAG) && !(TEST)) { return false; }
bool Filter::hasQuery() const
{
  return ACTIVE(QUERY);
}

bool Filter::hasName() const
{
  return ACTIVE(NAME);
}

bool Filter::hasCategory() const
{
  return ACTIVE(CATEGORY);
}

bool Filter::hasLang() const
{
  return ACTIVE(LANG);
}

bool Filter::hasPublisher() const
{
  return ACTIVE(_PUBLISHER);
}

bool Filter::hasCreator() const
{
  return ACTIVE(_CREATOR);
}

bool Filter::accept(const Book& book) const
{
  auto local = !book.getPath().empty();
  FILTER(_LOCAL, local)
  FILTER(_NOLOCAL, !local)

  auto valid = book.isPathValid();
  FILTER(_VALID, valid)
  FILTER(_NOVALID, !valid)

  auto remote = !book.getUrl().empty();
  FILTER(_REMOTE, remote)
  FILTER(_NOREMOTE, !remote)

  FILTER(MAXSIZE, book.getSize() <= _maxSize)

  return true;
}

}
