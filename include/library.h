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

#ifndef KIWIX_LIBRARY_H
#define KIWIX_LIBRARY_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "book.h"
#include "bookmark.h"
#include "common.h"

#define KIWIX_LIBRARY_VERSION "20110515"

namespace kiwix
{

class OPDSDumper;

enum supportedListSortBy { UNSORTED, TITLE, SIZE, DATE, CREATOR, PUBLISHER };
enum supportedListMode {
  ALL = 0,
  LOCAL = 1,
  REMOTE = 1 << 1,
  NOLOCAL = 1 << 2,
  NOREMOTE = 1 << 3,
  VALID = 1 << 4,
  NOVALID = 1 << 5
};

class Filter {
  private:
    uint64_t activeFilters;
    std::vector<std::string> _acceptTags;
    std::vector<std::string> _rejectTags;
    std::string _category;
    std::string _lang;
    std::string _publisher;
    std::string _creator;
    size_t _maxSize;
    std::string _query;
    bool _queryIsPartial;
    std::string _name;

  public:
    Filter();
    ~Filter() = default;

    /**
     *  Set the filter to check local.
     *
     *  A local book is a book with a path.
     *  If accept is true, only local book are accepted.
     *  If accept is false, only non local book are accepted.
     */
    Filter& local(bool accept);

    /**
     *  Set the filter to check remote.
     *
     *  A remote book is a book with a url.
     *  If accept is true, only remote book are accepted.
     *  If accept is false, only non remote book are accepted.
     */
    Filter& remote(bool accept);

    /**
     *  Set the filter to check validity.
     *
     *  A valid book is a book with a path pointing to a existing zim file.
     *  If accept is true, only valid book are accepted.
     *  If accept is false, only non valid book are accepted.
     */
    Filter& valid(bool accept);

    /**
     * Set the filter to only accept book with corresponding tag.
     */
    Filter& acceptTags(std::vector<std::string> tags);
    Filter& rejectTags(std::vector<std::string> tags);

    Filter& category(std::string category);
    Filter& lang(std::string lang);
    Filter& publisher(std::string publisher);
    Filter& creator(std::string creator);
    Filter& maxSize(size_t size);
    Filter& query(std::string query, bool partial=true);
    Filter& name(std::string name);

    bool hasQuery() const;
    const std::string& getQuery() const { return _query; }
    bool queryIsPartial() const { return _queryIsPartial; }

    bool accept(const Book& book) const;
    bool acceptByQueryOnly(const Book& book) const;
    bool acceptByNonQueryCriteria(const Book& book) const;
};


/**
 * A Library store several books.
 */
class Library
{
  std::map<std::string, kiwix::Book> m_books;
  std::map<std::string, std::shared_ptr<Reader>> m_readers;
  std::vector<kiwix::Bookmark> m_bookmarks;
  class BookDB;
  std::unique_ptr<BookDB> m_bookDB;

 public:
  typedef std::vector<std::string> BookIdCollection;

 public:
  Library();
  ~Library();

  /**
   * Library is not a copiable object. However it can be moved.
   */
  Library(const Library& ) = delete;
  Library(Library&& );
  void operator=(const Library& ) = delete;
  Library& operator=(Library&& );

  /**
   * Add a book to the library.
   *
   * If a book already exist in the library with the same id, update
   * the existing book instead of adding a new one.
   *
   * @param book The book to add.
   * @return True if the book has been added.
   *         False if a book has been updated.
   */
  bool addBook(const Book& book);

  /**
   * Add a bookmark to the library.
   *
   * @param bookmark the book to add.
   */
  void addBookmark(const Bookmark& bookmark);

  /**
   * Remove a bookmarkk
   *
   * @param zimId The zimId of the bookmark.
   * @param url The url of the bookmark.
   * @return True if the bookmark has been removed.
   */
  bool removeBookmark(const std::string& zimId, const std::string& url);

  Book& getBookById(const std::string& id);
  Book& getBookByPath(const std::string& path);
  std::shared_ptr<Reader> getReaderById(const std::string& id);

  /**
   * Remove a book from the library.
   *
   * @param id the id of the book to remove.
   * @return True if the book were in the lirbrary and has been removed.
   */
  bool removeBookById(const std::string& id);

  /**
   * Write the library to a file.
   *
   * @param path the path of the file to write to.
   * @return True if the library has been correctly saved.
   */
  bool writeToFile(const std::string& path);

  /**
   * Write the library bookmarks to a file.
   *
   * @param path the path of the file to write to.
   * @return True if the library has been correctly saved.
   */
  bool writeBookmarksToFile(const std::string& path);

  /**
   * Get the number of book in the library.
   *
   * @param localBooks If we must count local books (books with a path).
   * @param remoteBooks If we must count remote books (books with an url)
   * @return The number of books.
   */
  unsigned int getBookCount(const bool localBooks, const bool remoteBooks);

  /**
   * Get all langagues of the books in the library.
   *
   * @return A list of languages.
   */
  std::vector<std::string> getBooksLanguages();

  /**
   * Get all book creators of the books in the library.
   *
   * @return A list of book creators.
   */
  std::vector<std::string> getBooksCreators();

  /**
   * Get all book publishers of the books in the library.
   *
   * @return A list of book publishers.
   */
  std::vector<std::string> getBooksPublishers();

  /**
   * Get all bookmarks.
   *
   * @return A list of bookmarks
   */
  const std::vector<kiwix::Bookmark> getBookmarks(bool onlyValidBookmarks = true);

  /**
   * Get all book ids of the books in the library.
   *
   * @return A list of book ids.
   */
  BookIdCollection getBooksIds();

  /**
   * Filter the library and generate a new one with the keep elements.
   *
   * This is equivalent to `listBookIds(ALL, UNSORTED, search)`.
   *
   * @param search List only books with search in the title or description.
   * @return The list of bookIds corresponding to the query.
   */
  DEPRECATED BookIdCollection filter(const std::string& search);


  /**
   * Filter the library and return the id of the keep elements.
   *
   * @param filter The filter to use.
   * @return The list of bookIds corresponding to the filter.
   */
  BookIdCollection filter(const Filter& filter);


  /**
   * Sort (in place) bookIds using the given comparator.
   *
   * @param bookIds the list of book Ids to sort
   * @param comparator how to sort the books
   * @return The sorted list of books
   */
  void sort(BookIdCollection& bookIds, supportedListSortBy sortBy, bool ascending);

  /**
   * List books in the library.
   *
   * @param mode The mode of listing :
   *             - LOCAL    : list only local books (with a path).
   *             - REMOTE   : list only remote books (with an url).
   *             - VALID    : list only valid books (without a path or with a
   *                          path pointing to a valid zim file).
   *             - NOLOCAL  : list only books without valid path.
   *             - NOREMOTE : list only books without url.
   *             - NOVALID  : list only books not valid.
   *             - ALL : Do not do any filter (LOCAL or REMOTE)
   *             - Flags can be combined.
   * @param sortBy Attribute to sort by the book list.
   * @param search List only books with search in the title, description.
   * @param language List only books in this language.
   * @param creator List only books of this creator.
   * @param publisher List only books of this publisher.
   * @param maxSize Do not list book bigger than maxSize.
   *                Set to 0 to cancel this filter.
   * @return The list of bookIds corresponding to the query.
   */
  DEPRECATED BookIdCollection listBooksIds(
    int supportedListMode = ALL,
    supportedListSortBy sortBy = UNSORTED,
    const std::string& search = "",
    const std::string& language = "",
    const std::string& creator = "",
    const std::string& publisher = "",
    const std::vector<std::string>& tags = {},
    size_t maxSize = 0);

  friend class OPDSDumper;
  friend class libXMLDumper;

private: // functions
  BookIdCollection getBooksByTitleOrDescription(const Filter& filter);
  void updateBookDB(const Book& book);
};

}

#endif
