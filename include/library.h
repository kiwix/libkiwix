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

#define KIWIX_LIBRARY_VERSION "20110515"

namespace kiwix
{

class Book;
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
/**
 * A Library store several books.
 */
class Library
{
  std::map<std::string, kiwix::Book> books;
 public:
  Library();
  ~Library();

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

  Book& getBookById(const std::string& id);

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
   * @return True if the library has been correctly save.
   */
  bool writeToFile(const std::string& path);

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
   * Get all book ids of the books in the library.
   *
   * @return A list of book ids.
   */
  std::vector<std::string> getBooksIds();

  /**
   * Filter the library and generate a new one with the keep elements.
   *
   * This is equivalent to `listBookIds(ALL, UNSORTED, search)`.
   *
   * @param search List only books with search in the title or description.
   * @return The list of bookIds corresponding to the query.
   */
  std::vector<std::string> filter(const std::string& search);


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
  std::vector<std::string> listBooksIds(
    int supportedListMode = ALL,
    supportedListSortBy sortBy = UNSORTED,
    const std::string& search = "",
    const std::string& language = "",
    const std::string& creator = "",
    const std::string& publisher = "",
    size_t maxSize = 0);

  friend class OPDSDumper;
  friend class libXMLDumper;
};
}

#endif
