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

#include "manager.h"

namespace kiwix {

  /* Constructor */
  Manager::Manager() :
    writableLibraryPath("") {
  }
  
  /* Destructor */
  Manager::~Manager() {
  }

  bool Manager::parseXmlDom(const pugi::xml_document &doc, const bool readOnly, const string libraryPath) {
    pugi::xml_node libraryNode = doc.child("library");

    if (strlen(libraryNode.attribute("current").value()))
      library.current = libraryNode.attribute("current").value();

    string libraryVersion = libraryNode.attribute("version").value();
    
    for (pugi::xml_node bookNode = libraryNode.child("book"); bookNode; bookNode = bookNode.next_sibling("book")) {
      bool ok = true;
      kiwix::Book book;

      book.readOnly = readOnly;
      book.id = bookNode.attribute("id").value();
      book.path = bookNode.attribute("path").value();
      book.last = (std::string(bookNode.attribute("last").value()) != "undefined" ? 
		   bookNode.attribute("last").value() : "");
      book.indexPath = bookNode.attribute("indexPath").value();
      book.indexType = (std::string(bookNode.attribute("indexType").value()) == "xapian" ? XAPIAN : CLUCENE);
      book.title = bookNode.attribute("title").value();
      book.description = bookNode.attribute("description").value();
      book.language = bookNode.attribute("language").value();
      book.date = bookNode.attribute("date").value();
      book.creator = bookNode.attribute("creator").value();
      book.url = bookNode.attribute("url").value();
      book.articleCount = bookNode.attribute("articleCount").value();
      book.mediaCount = bookNode.attribute("mediaCount").value();
      book.size = bookNode.attribute("size").value();
      book.favicon = bookNode.attribute("favicon").value();
      book.faviconMimeType = bookNode.attribute("faviconMimeType").value();
      
      /* Compute absolute paths if relative one are used */
      if (!book.path.empty() && isRelativePath(book.path))
	book.pathAbsolute = computeAbsolutePath(libraryPath, book.path);
      else
	book.pathAbsolute = book.path;

      if (book.indexPath.empty() && isRelativePath(book.indexPath))
	book.indexPathAbsolute = computeAbsolutePath(libraryPath, book.indexPath);
      else
	book.indexPathAbsolute = book.indexPath;

      /* Update the book properties with the new importer */
      if (libraryVersion.empty() || atoi(libraryVersion.c_str()) < atoi(KIWIX_LIBRARY_VERSION)) {
	if (!book.path.empty()) {
	  ok = this->readBookFromPath(book.pathAbsolute, book);
	}
      }

      if (ok) {
	library.addBook(book);
      }
    }
    
    return true;
  }

  bool Manager::isRelativePath(const string &path) {
#ifdef _WIN32
    return path.substr(1, 2) == ":\\" ? false : true;
#else
    return path.substr(0, 1) == "/" ? false : true;
#endif
  }

  string Manager::computeAbsolutePath(const string libraryPath, const string relativePath) {
#ifdef _WIN32
    string separator = "\\";
#else
    string separator = "/";
#endif
    string absolutePath = removeLastPathElement(libraryPath, true, false);
    char *cRelativePath = strdup(relativePath.c_str());
    char *token = strtok(cRelativePath, separator.c_str());
    while (token != NULL) {
      if (string(token) == "..") {
	absolutePath = removeLastPathElement(absolutePath, true, false);
	token = strtok(NULL, separator.c_str());
      } else if (token != "." && token != "") {
	absolutePath += string(token);
	token = strtok(NULL, separator.c_str());
	if (token != NULL)
	  absolutePath += separator;
      } else {
	token = strtok(NULL, separator.c_str());
      }
    }

    return absolutePath;
  }

  string Manager::removeLastPathElement(const string path, const bool removePreSeparator, const bool removePostSeparator) {
#ifdef _WIN32
    string separator = "\\";
#else
    string separator = "/";
#endif
    
    string newPath = path;
    size_t offset = newPath.find_last_of(separator);
    if (removePreSeparator && offset != newPath.find_first_of(separator) && offset == newPath.length()-1) {
      newPath = newPath.substr(0, offset);
      offset = newPath.find_last_of(separator);
    }
    newPath = removePostSeparator ? newPath.substr(0, offset) : newPath.substr(0, offset+1);
    return newPath;
  }
  
  bool Manager::readXml(const string xml, const bool readOnly, const string libraryPath) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer_inplace((void*)xml.data(), xml.size());

    if (result) {
      this->parseXmlDom(doc, readOnly, libraryPath);
    }

    return true;
  }

  bool Manager::readFile(const string path, const bool readOnly) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());

    if (result) {
      this->parseXmlDom(doc, readOnly, path);
    }

    if (!readOnly) {
      this->writableLibraryPath = path;
    }

    return true;
  }

  bool Manager::writeFile(const string path) {
    pugi::xml_document doc;

    /* Add the library node */
    pugi::xml_node libraryNode = doc.append_child("library");

    if (library.current != "") {
      libraryNode.append_attribute("current") = library.current.c_str();
    }

    if (library.version != "")
      libraryNode.append_attribute("version") = library.version.c_str();
    
    /* Add each book */
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {

      if (!itr->readOnly) {
	pugi::xml_node bookNode = libraryNode.append_child("book");
	bookNode.append_attribute("id") = itr->id.c_str();

	if (itr->path != "")
	  bookNode.append_attribute("path") = itr->path.c_str();
	
	if (itr->last != "" && itr->last != "undefined") {
	  bookNode.append_attribute("last") = itr->last.c_str();
	}
	
	if (itr->indexPath != "") {
	  bookNode.append_attribute("indexPath") = itr->indexPath.c_str();
	  if (itr->indexType == XAPIAN)
	    bookNode.append_attribute("indexType") = "xapian";
	  else if (itr->indexType == CLUCENE)
	    bookNode.append_attribute("indexType") = "clucene";
	}
	
	if (itr->title != "")
	  bookNode.append_attribute("title") = itr->title.c_str();
	
	if (itr->description != "")
	  bookNode.append_attribute("description") = itr->description.c_str();
	
	if (itr->language != "")
	  bookNode.append_attribute("language") = itr->language.c_str();
	
	if (itr->date != "")
	  bookNode.append_attribute("date") = itr->date.c_str();
	
	if (itr->creator != "")
	  bookNode.append_attribute("creator") = itr->creator.c_str();
	
	if (itr->url != "")
	  bookNode.append_attribute("url") = itr->url.c_str();
	
	if (itr->articleCount != "")
	  bookNode.append_attribute("articleCount") = itr->articleCount.c_str();
	
	if (itr->mediaCount != "")
	  bookNode.append_attribute("mediaCount") = itr->mediaCount.c_str();

	if (itr->size != "")
	  bookNode.append_attribute("size") = itr->size.c_str();

	if (itr->favicon != "")
	  bookNode.append_attribute("favicon") = itr->favicon.c_str();

	if (itr->faviconMimeType != "")
	  bookNode.append_attribute("faviconMimeType") = itr->faviconMimeType.c_str();
      }
    }

    /* saving file */
    doc.save_file(path.c_str());

    return true;
  }

  bool Manager::setCurrentBookId(const string id) {
    library.current = id;
    return true;
  }

  string Manager::getCurrentBookId() {
    return library.current;
  }

  bool Manager::addBookFromPath(const string path, const string url, const bool checkMetaData) {
    kiwix::Book book;
    
    if (this->readBookFromPath(path, book)) {

      if (!checkMetaData || 
	  checkMetaData && !book.title.empty() && !book.language.empty() && !book.date.empty()) {
	book.url = url;
	library.addBook(book);
	return true;
      }
    }

    return false;
  }
  
  bool Manager::readBookFromPath(const string path, kiwix::Book &book) {

    try {
      kiwix::Reader reader = kiwix::Reader(path);
      book.path = path;
      book.pathAbsolute = path;
      book.id = reader.getId();
      book.title = reader.getTitle();
      book.description = reader.getDescription();
      book.language = reader.getLanguage();
      book.date = reader.getDate();
      book.creator = reader.getCreator();
      
      std::ostringstream articleCountStream;
      articleCountStream << reader.getArticleCount();
      book.articleCount = articleCountStream.str();
      
      std::ostringstream mediaCountStream;
      mediaCountStream << reader.getMediaCount();
      book.mediaCount = mediaCountStream.str();
      
      struct stat filestatus;
      stat( path.c_str(), &filestatus );
      unsigned int size = filestatus.st_size / 1024;
      char csize[42];
      sprintf (csize, "%u", size);
      book.size = csize;

      string favicon;
      string faviconMimeType;
      if (reader.getFavicon(favicon, faviconMimeType)) {
	book.favicon = base64_encode(reinterpret_cast<const unsigned char*>(favicon.c_str()), favicon.length());
	book.faviconMimeType = faviconMimeType;
      }
    } catch (...) {
      return false;
    }

    return true;
  }

  bool Manager::removeBookByIndex(const unsigned int bookIndex) {
    return this->library.removeBookByIndex(bookIndex);
  }

  bool Manager::removeBookById(const string id) {
    unsigned int bookIndex = 0;
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {    
      if ( itr->id == id) {
	return this->library.removeBookByIndex(bookIndex);
      }
      bookIndex++;
    }
    return false;
  }

  kiwix::Library Manager::cloneLibrary() {
    return this->library;
  }

  bool Manager::getBookById(const string id, Book &book) {
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {    
      if ( itr->id == id) {
	book = *itr;
	return true;
      }
    }
    return false;
  }

  bool Manager::updateBookLastOpenDateById(const string id) {
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {    
      if ( itr->id == id) {
	char unixdate[12];
	sprintf (unixdate, "%d", (int)time(NULL));
	itr->last = unixdate;
	return true;
      }
    }

    return false;
  }

  bool Manager::setBookIndex(const string id, const string path, const supportedIndexType type) {
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {    
      if ( itr->id == id) {
	itr->indexPath = path;
	itr->indexPathAbsolute = path;
	itr->indexType = type;
	return true;
      }
    }

    return false;
  }

  bool Manager::setBookPath(const string id, const string path) {
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {    
      if ( itr->id == id) {
	itr->path = path;
	itr->pathAbsolute = path;
	return true;
      }
    }

    return false;
  }

  void Manager::removeBookPaths() {
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {    
      itr->path = "";
      itr->pathAbsolute = "";
    }
  }

  unsigned int Manager::getBookCount(const bool localBooks, const bool remoteBooks) {
    unsigned int result = 0;
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
      if (!itr->path.empty() && localBooks || itr->path.empty() && remoteBooks)
	result++;
    }
    return result;
  }

  bool Manager::listBooks(const supportedListMode mode) {
    this->bookIdList.clear();
    std::vector<kiwix::Book>::iterator itr;

    if (mode == LASTOPEN) {
      std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortByLastOpen);
      for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
	if (!itr->last.empty())
	this->bookIdList.push_back(itr->id);
      }
    } else if (mode == REMOTE) {
      for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
	if (itr->path.empty() && !itr->url.empty()) {
	  this->bookIdList.push_back(itr->id);
	}
      }
    } else {
      for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
	if (!itr->path.empty())
	  this->bookIdList.push_back(itr->id);
      }
    }

    return true;
  }

}
