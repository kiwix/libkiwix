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

#ifndef KIWIX_MANAGER_H
#define KIWIX_MANAGER_H

#include <string>
#include <sstream>
#include <time.h>

#include <pugixml.hpp>

#include "../base64.h"
#include "../regexTools.h"
#include "../pathTools.h"
#include <kiwix/library.h>
#include <kiwix/reader.h>

using namespace std;

namespace kiwix {

  enum supportedListMode { LASTOPEN, REMOTE, LOCAL };
  enum supportedListSortBy { TITLE, SIZE, DATE, CREATOR, PUBLISHER };

  class Manager {
    
  public:
    Manager();
    ~Manager();

    bool readFile(const string path, const bool readOnly = true);
    bool readXml(const string xml, const bool readOnly = true, const string libraryPath = "");
    bool writeFile(const string path);
    bool removeBookByIndex(const unsigned int bookIndex);
    bool removeBookById(const string id);
    bool setCurrentBookId(const string id);
    string getCurrentBookId();
    bool setBookIndex(const string id, const string path, const supportedIndexType type);
    bool setBookPath(const string id, const string path);
    string addBookFromPathAndGetId(const string pathToOpen, const string pathToSave = "", const string url = "", 
				   const bool checkMetaData = false);
    bool addBookFromPath(const string pathToOpen, const string pathToSave = "", const string url = "", 
			 const bool checkMetaData = false);
    Library cloneLibrary();
    bool getBookById(const string id, Book &book);
    bool getCurrentBook(Book &book);
    unsigned int getBookCount(const bool localBooks, const bool remoteBooks);
    bool updateBookLastOpenDateById(const string id);
    void removeBookPaths();
    bool listBooks(const supportedListMode mode, const supportedListSortBy sortBy, const unsigned int maxSize, 
		   const string language, const string creator, const string publisher, const string search);
    vector<string> getBooksLanguages();
    vector<string> getBooksCreators();
    vector<string> getBooksPublishers();
    vector<string> getBooksIds();

    string writableLibraryPath;

    vector<std::string> bookIdList;
    
  protected:
    kiwix::Library library;
    
    bool readBookFromPath(const string path, Book &book);
    bool parseXmlDom(const pugi::xml_document &doc, const bool readOnly, const string libraryPath);
  };

}

#endif
