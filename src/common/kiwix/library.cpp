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
    readOnly(false) {
  }
  
  /* Destructor */
  Book::~Book() {
  }

  /* Sort functions */
  bool Book::sortByLastOpen(const kiwix::Book &a, const kiwix::Book &b) {
    return atoi(a.last.c_str()) > atoi(b.last.c_str());
  }

  bool Book::sortByTitle(const kiwix::Book &a, const kiwix::Book &b) {
    return strcmp(a.title.c_str(), b.title.c_str()) < 0;
  }

  bool Book::sortByDate(const kiwix::Book &a, const kiwix::Book &b) {
    return strcmp(a.date.c_str(), b.date.c_str()) > 0;
  }

  bool Book::sortBySize(const kiwix::Book &a, const kiwix::Book &b) {
    return atoi(a.size.c_str()) < atoi(b.size.c_str());
  }

  bool Book::sortByPublisher(const kiwix::Book &a, const kiwix::Book &b) {
    return strcmp(a.publisher.c_str(), b.publisher.c_str()) < 0;
  }

  bool Book::sortByCreator(const kiwix::Book &a, const kiwix::Book &b) {
    return strcmp(a.creator.c_str(), b.creator.c_str()) < 0;
  }

  bool Book::sortByLanguage(const kiwix::Book &a, const kiwix::Book &b) {
    return strcmp(a.language.c_str(), b.language.c_str()) < 0;
  }

  std::string Book::getHumanReadableIdFromPath() {
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
  Library::Library():
    version(KIWIX_LIBRARY_VERSION) {
  }
  
  /* Destructor */
  Library::~Library() {
  }

  bool Library::addBook(const Book &book) {

    /* Try to find it */
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = this->books.begin(); itr != this->books.end(); ++itr ) {
      if (itr->id == book.id && (itr->readOnly == book.readOnly || book.readOnly)) {

	itr->readOnly = book.readOnly;

	if (itr->path.empty()) 
	  itr->path = book.path;

	if (itr->pathAbsolute.empty()) 
	  itr->pathAbsolute = book.pathAbsolute;

	if (itr->url.empty())
	  itr->url = book.url;

	if (itr->indexPath.empty()) {
	  itr->indexPath = book.indexPath;
	  itr->indexType = book.indexType;
	}

	if (itr->indexPathAbsolute.empty()) {
	  itr->indexPathAbsolute = book.indexPathAbsolute;
	  itr->indexType = book.indexType;
	}

	if (itr->faviconMimeType.empty()) {
	  itr->favicon = book.favicon;
	  itr->faviconMimeType = book.faviconMimeType;
	}

	return false;
      }
    }

    /* otherwise */
    this->books.push_back(book);
    return true;
  }

  bool Library::removeBookByIndex(const unsigned int bookIndex) {
    books.erase(books.begin()+bookIndex);
    return true;
  }

}
