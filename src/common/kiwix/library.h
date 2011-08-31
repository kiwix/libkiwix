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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define KIWIX_LIBRARY_VERSION "20110515"

using namespace std;

namespace kiwix {

  enum supportedIndexType { UNKNOWN, XAPIAN, CLUCENE };

  class Book {

  public:
    Book();
    ~Book();

    static bool sortByLastOpen(const Book &a, const Book &b);

    string id;
    string path;
    string pathAbsolute;
    string last;
    string indexPath;
    string indexPathAbsolute;
    supportedIndexType indexType;
    string title;
    string description;
    string language;
    string creator;
    string date;
    string url;
    string articleCount;
    string mediaCount;
    bool readOnly;
    string size;
    string favicon;
    string faviconMimeType;
  };

  class Library {
    
  public:
    Library();
    ~Library();

    string current;
    string version;
    bool addBook(const Book &book);
    bool removeBookByIndex(const unsigned int bookIndex);
    vector <kiwix::Book> books;

  };

}

#endif
