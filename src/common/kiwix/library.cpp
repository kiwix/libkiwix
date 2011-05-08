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

namespace kiwix {

  /* Constructor */
  Book::Book():
    id(""),
    path(""),
    last(""),
    indexPath(""),
    indexType(UNKNOWN),
    title(""),
    description(""),
    language(""),
    date(""),
    creator(""),
    url(""),
    articleCount(""),
    mediaCount(""),
    readOnly(false),
    size("") {
  }
  
  /* Destructor */
  Book::~Book() {
  }

  bool Book::sortByLastOpen(const kiwix::Book &a, const kiwix::Book &b) {
    return atoi(a.last.c_str()) > atoi(b.last.c_str());
  }

  /* Constructor */
  Library::Library():
    current(""),
    version(KIWIX_LIBRARY_VERSION) {
  }
  
  /* Destructor */
  Library::~Library() {
  }

  bool Library::addBook(const Book &book) {
    this->books.push_back(book);
    return true;
  }

  bool Library::removeBookByIndex(const unsigned int bookIndex) {
    books.erase(books.begin()+bookIndex-1);
    return true;
  }

}
