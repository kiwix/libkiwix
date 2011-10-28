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
      book.pathAbsolute = isRelativePath(book.path) ?
	computeAbsolutePath(libraryPath, book.path) : book.path;
      book.indexPathAbsolute = isRelativePath(book.indexPath) ?
	computeAbsolutePath(libraryPath, book.indexPath) : book.indexPath;

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
      
      if (!readOnly) {
	this->writableLibraryPath = path;
      }
    }

    return true;
  }

  bool Manager::writeFile(const string path) {
    pugi::xml_document doc;

    /* Add the library node */
    pugi::xml_node libraryNode = doc.append_child("library");

    if (!library.current.empty()) {
      libraryNode.append_attribute("current") = library.current.c_str();
    }

    if (!library.version.empty())
      libraryNode.append_attribute("version") = library.version.c_str();
    
    /* Add each book */
    std::vector<kiwix::Book>::iterator itr;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {

      if (!itr->readOnly) {
	pugi::xml_node bookNode = libraryNode.append_child("book");
	bookNode.append_attribute("id") = itr->id.c_str();

	if (!itr->path.empty())
	  bookNode.append_attribute("path") = itr->path.c_str();
	
	if (!itr->last.empty() && itr->last != "undefined") {
	  bookNode.append_attribute("last") = itr->last.c_str();
	}
	
	if (!itr->indexPath.empty()) {
	  bookNode.append_attribute("indexPath") = itr->indexPath.c_str();
	  if (itr->indexType == XAPIAN)
	    bookNode.append_attribute("indexType") = "xapian";
	  else if (itr->indexType == CLUCENE)
	    bookNode.append_attribute("indexType") = "clucene";
	}
	
	if (!itr->title.empty())
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

  bool Manager::addBookFromPath(const string pathToOpen, const string pathToSave, const string url, const bool checkMetaData) {
    kiwix::Book book;

    if (this->readBookFromPath(pathToOpen, book)) {

      if (!pathToSave.empty() && pathToSave != pathToOpen) {
	book.path = pathToSave;
	book.pathAbsolute = pathToSave;
      }

      if (!checkMetaData || 
	  (checkMetaData && !book.title.empty() && !book.language.empty() && !book.date.empty())) {
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
      
      book.size = getFileSizeAsString(path);

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

  vector<string> Manager::getBooksLanguages() {
    std::vector<string> booksLanguages;
    std::vector<kiwix::Book>::iterator itr;
    std::map<string, bool> booksLanguagesMap;

    std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortByLanguage);      
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
      if (booksLanguagesMap.find(itr->language) == booksLanguagesMap.end()) {
	booksLanguagesMap[itr->language] = true;
	booksLanguages.push_back(itr->language);
      }
    }
    
    return booksLanguages;
  }

  vector<string> Manager::getBooksPublishers() {
    std::vector<string> booksPublishers;
    std::vector<kiwix::Book>::iterator itr;
    std::map<string, bool> booksPublishersMap;

    std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortByPublisher);      
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
      if (booksPublishersMap.find(itr->creator) == booksPublishersMap.end()) {
	booksPublishersMap[itr->creator] = true;
	booksPublishers.push_back(itr->creator);
      }
    }
    
    return booksPublishers;
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
      if ((!itr->path.empty() && localBooks) || (itr->path.empty() && remoteBooks))
	result++;
    }
    return result;
  }

  bool Manager::listBooks(const supportedListMode mode, const supportedListSortBy sortBy, const unsigned int maxSize,
			  const string language, const string publisher, const string search) {
    this->bookIdList.clear();
    std::vector<kiwix::Book>::iterator itr;

    /* Sort */
    if (sortBy == TITLE) {
      std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortByTitle);      
    } else if (sortBy == SIZE) {
      std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortBySize);
    } else if (sortBy == DATE) {
      std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortByDate);
    } else if (sortBy == PUBLISHER) {
      std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortByPublisher);
    }
    
    /* Special sort for LASTOPEN */
    if (mode == LASTOPEN) {
      std::sort(library.books.begin(), library.books.end(), kiwix::Book::sortByLastOpen);
      for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
	if (!itr->last.empty())
	  this->bookIdList.push_back(itr->id);
      }
    } else {
      /* Generate the list of book id */
      for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
	bool ok = true;
	
	if (mode == LOCAL && itr->path.empty())
	  ok = false;
	
	if (ok == true && mode == REMOTE && (!itr->path.empty() || itr->url.empty()))
	  ok = false;
	
	if (ok == true && (unsigned int)atoi(itr->size.c_str()) > maxSize * 1024 * 1024)
	  ok = false;
	
	if (ok == true && !language.empty() && itr->language != language)
	  ok = false;
	
	if (ok == true && !publisher.empty() && itr->creator != publisher)
	  ok = false;
	
	if ((ok == true && !search.empty()) && !(matchRegex(itr->title, search) || matchRegex(itr->description, search)))
	  ok = false;

	if (ok == true) {
	  this->bookIdList.push_back(itr->id);
	}
      }
    }
    
    return true;
  }

}
