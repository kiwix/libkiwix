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

namespace kiwix
{
/* Constructor */
Manager::Manager() : writableLibraryPath("")
{
}
/* Destructor */
Manager::~Manager()
{
}
bool Manager::parseXmlDom(const pugi::xml_document& doc,
                          const bool readOnly,
                          const string libraryPath)
{
  pugi::xml_node libraryNode = doc.child("library");

  if (strlen(libraryNode.attribute("current").value()))
    this->setCurrentBookId(libraryNode.attribute("current").value());

  string libraryVersion = libraryNode.attribute("version").value();

  for (pugi::xml_node bookNode = libraryNode.child("book"); bookNode;
       bookNode = bookNode.next_sibling("book")) {
    bool ok = true;
    kiwix::Book book;

    book.readOnly = readOnly;
    book.id = bookNode.attribute("id").value();
    book.path = bookNode.attribute("path").value();
    book.last = (std::string(bookNode.attribute("last").value()) != "undefined"
                     ? bookNode.attribute("last").value()
                     : "");
    book.indexPath = bookNode.attribute("indexPath").value();
    book.indexType = XAPIAN;
    book.title = bookNode.attribute("title").value();
    book.name = bookNode.attribute("name").value();
    book.tags = bookNode.attribute("tags").value();
    book.description = bookNode.attribute("description").value();
    book.language = bookNode.attribute("language").value();
    book.date = bookNode.attribute("date").value();
    book.creator = bookNode.attribute("creator").value();
    book.publisher = bookNode.attribute("publisher").value();
    book.url = bookNode.attribute("url").value();
    book.origId = bookNode.attribute("origId").value();
    book.articleCount = bookNode.attribute("articleCount").value();
    book.mediaCount = bookNode.attribute("mediaCount").value();
    book.size = bookNode.attribute("size").value();
    book.favicon = bookNode.attribute("favicon").value();
    book.faviconMimeType = bookNode.attribute("faviconMimeType").value();

    /* Check absolute and relative paths */
    this->checkAndCleanBookPaths(book, libraryPath);

    /* Update the book properties with the new importer */
    if (libraryVersion.empty()
        || atoi(libraryVersion.c_str()) <= atoi(KIWIX_LIBRARY_VERSION)) {
      if (!book.path.empty()) {
        ok = this->readBookFromPath(book.pathAbsolute);
      }
    }

    if (ok) {
      library.addBook(book);
    }
  }

  return true;
}

bool Manager::readXml(const string& xml,
                      const bool readOnly,
                      const string libraryPath)
{
  pugi::xml_document doc;
  pugi::xml_parse_result result
      = doc.load_buffer_inplace((void*)xml.data(), xml.size());

  if (result) {
    this->parseXmlDom(doc, readOnly, libraryPath);
  }

  return true;
}

bool Manager::readFile(const string path, const bool readOnly)
{
  return this->readFile(path, path, readOnly);
}

bool Manager::readFile(const string nativePath,
                       const string UTF8Path,
                       const bool readOnly)
{
  bool retVal = true;
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(nativePath.c_str());

  if (result) {
    this->parseXmlDom(doc, readOnly, UTF8Path);
  } else {
    retVal = false;
  }

  /* This has to be set (although if the file does not exists) to be
   * able to know where to save the library if new content are
   * available */
  if (!readOnly) {
    this->writableLibraryPath = UTF8Path;
  }

  return retVal;
}

bool Manager::writeFile(const string path)
{
  pugi::xml_document doc;

  /* Add the library node */
  pugi::xml_node libraryNode = doc.append_child("library");

  if (!getCurrentBookId().empty()) {
    libraryNode.append_attribute("current") = getCurrentBookId().c_str();
  }

  if (!library.version.empty())
    libraryNode.append_attribute("version") = library.version.c_str();

  /* Add each book */
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (!itr->readOnly) {
      this->checkAndCleanBookPaths(*itr, path);

      pugi::xml_node bookNode = libraryNode.append_child("book");
      bookNode.append_attribute("id") = itr->id.c_str();

      if (!itr->path.empty()) {
        bookNode.append_attribute("path") = itr->path.c_str();
      }

      if (!itr->last.empty() && itr->last != "undefined") {
        bookNode.append_attribute("last") = itr->last.c_str();
      }

      if (!itr->indexPath.empty())
        bookNode.append_attribute("indexPath") = itr->indexPath.c_str();

      if (!itr->indexPath.empty() || !itr->indexPathAbsolute.empty()) {
        if (itr->indexType == XAPIAN) {
          bookNode.append_attribute("indexType") = "xapian";
        }
      }

      if (itr->origId.empty()) {
        if (!itr->title.empty())
          bookNode.append_attribute("title") = itr->title.c_str();

        if (!itr->name.empty())
          bookNode.append_attribute("name") = itr->name.c_str();

        if (!itr->tags.empty())
          bookNode.append_attribute("tags") = itr->tags.c_str();

        if (!itr->description.empty())
          bookNode.append_attribute("description") = itr->description.c_str();

        if (!itr->language.empty())
          bookNode.append_attribute("language") = itr->language.c_str();

        if (!itr->creator.empty())
          bookNode.append_attribute("creator") = itr->creator.c_str();

        if (!itr->publisher.empty())
          bookNode.append_attribute("publisher") = itr->publisher.c_str();

        if (!itr->favicon.empty())
          bookNode.append_attribute("favicon") = itr->favicon.c_str();

        if (!itr->faviconMimeType.empty())
          bookNode.append_attribute("faviconMimeType")
              = itr->faviconMimeType.c_str();
      }

      if (!itr->date.empty()) {
        bookNode.append_attribute("date") = itr->date.c_str();
      }

      if (!itr->url.empty()) {
        bookNode.append_attribute("url") = itr->url.c_str();
      }

      if (!itr->origId.empty())
        bookNode.append_attribute("origId") = itr->origId.c_str();

      if (!itr->articleCount.empty())
        bookNode.append_attribute("articleCount") = itr->articleCount.c_str();

      if (!itr->mediaCount.empty())
        bookNode.append_attribute("mediaCount") = itr->mediaCount.c_str();

      if (!itr->size.empty()) {
        bookNode.append_attribute("size") = itr->size.c_str();
      }
    }
  }

  /* saving file */
  doc.save_file(path.c_str());

  return true;
}


bool Manager::setCurrentBookId(const string id)
{
  if (library.current.empty() || library.current.top() != id) {
    if (id.empty() && !library.current.empty()) {
      library.current.pop();
    } else {
      library.current.push(id);
    }
  }
  return true;
}

string Manager::getCurrentBookId() const
{
  return library.current.empty() ? "" : library.current.top();
}

/* Add a book to the library. Return empty string if failed, book id otherwise
 */
string Manager::addBookFromPathAndGetId(const string pathToOpen,
                                        const string pathToSave,
                                        const string url,
                                        const bool checkMetaData)
{
  kiwix::Book book;

  if (this->readBookFromPath(pathToOpen, &book)) {
    if (pathToSave != pathToOpen) {
      book.path = pathToSave;
      book.pathAbsolute
          = isRelativePath(pathToSave)
                ? computeAbsolutePath(
                      removeLastPathElement(writableLibraryPath, true, false),
                      pathToSave)
                : pathToSave;
    }

    if (!checkMetaData
        || (checkMetaData && !book.title.empty() && !book.language.empty()
            && !book.date.empty())) {
      book.url = url;
      library.addBook(book);
      return book.id;
    }
  }

  return "";
}

/* Wrapper over Manager::addBookFromPath which return a bool instead of a string
 */
bool Manager::addBookFromPath(const string pathToOpen,
                              const string pathToSave,
                              const string url,
                              const bool checkMetaData)
{
  return !(
      this->addBookFromPathAndGetId(pathToOpen, pathToSave, url, checkMetaData)
          .empty());
}

bool Manager::readBookFromPath(const string path, kiwix::Book* book)
{
  try {
    kiwix::Reader* reader = new kiwix::Reader(path);

    if (book != NULL) {
      book->path = path;
      book->pathAbsolute = path;
      book->id = reader->getId();
      book->description = reader->getDescription();
      book->language = reader->getLanguage();
      book->date = reader->getDate();
      book->creator = reader->getCreator();
      book->publisher = reader->getPublisher();
      book->title = reader->getTitle();
      book->name = reader->getName();
      book->tags = reader->getTags();
      book->origId = reader->getOrigId();
      std::ostringstream articleCountStream;
      articleCountStream << reader->getArticleCount();
      book->articleCount = articleCountStream.str();

      std::ostringstream mediaCountStream;
      mediaCountStream << reader->getMediaCount();
      book->mediaCount = mediaCountStream.str();

      ostringstream convert;
      convert << reader->getFileSize();
      book->size = convert.str();

      string favicon;
      string faviconMimeType;
      if (reader->getFavicon(favicon, faviconMimeType)) {
        book->favicon = base64_encode(
            reinterpret_cast<const unsigned char*>(favicon.c_str()),
            favicon.length());
        book->faviconMimeType = faviconMimeType;
      }
    }

    delete reader;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}

bool Manager::removeBookByIndex(const unsigned int bookIndex)
{
  return this->library.removeBookByIndex(bookIndex);
}

bool Manager::removeBookById(const string id)
{
  unsigned int bookIndex = 0;
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (itr->id == id) {
      return this->library.removeBookByIndex(bookIndex);
    }
    bookIndex++;
  }
  return false;
}

vector<string> Manager::getBooksLanguages()
{
  std::vector<string> booksLanguages;
  std::vector<kiwix::Book>::iterator itr;
  std::map<string, bool> booksLanguagesMap;

  std::sort(
      library.books.begin(), library.books.end(), kiwix::Book::sortByLanguage);
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (booksLanguagesMap.find(itr->language) == booksLanguagesMap.end()) {
      if (itr->origId.empty()) {
        booksLanguagesMap[itr->language] = true;
        booksLanguages.push_back(itr->language);
      }
    }
  }

  return booksLanguages;
}

vector<string> Manager::getBooksCreators()
{
  std::vector<string> booksCreators;
  std::vector<kiwix::Book>::iterator itr;
  std::map<string, bool> booksCreatorsMap;

  std::sort(
      library.books.begin(), library.books.end(), kiwix::Book::sortByCreator);
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (booksCreatorsMap.find(itr->creator) == booksCreatorsMap.end()) {
      if (itr->origId.empty()) {
        booksCreatorsMap[itr->creator] = true;
        booksCreators.push_back(itr->creator);
      }
    }
  }

  return booksCreators;
}

vector<string> Manager::getBooksIds()
{
  std::vector<string> booksIds;
  std::vector<kiwix::Book>::iterator itr;

  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    booksIds.push_back(itr->id);
  }

  return booksIds;
}

vector<string> Manager::getBooksPublishers()
{
  std::vector<string> booksPublishers;
  std::vector<kiwix::Book>::iterator itr;
  std::map<string, bool> booksPublishersMap;

  std::sort(
      library.books.begin(), library.books.end(), kiwix::Book::sortByPublisher);
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (booksPublishersMap.find(itr->publisher) == booksPublishersMap.end()) {
      if (itr->origId.empty()) {
        booksPublishersMap[itr->publisher] = true;
        booksPublishers.push_back(itr->publisher);
      }
    }
  }

  return booksPublishers;
}

kiwix::Library Manager::cloneLibrary()
{
  return this->library;
}
bool Manager::getCurrentBook(Book& book)
{
  string currentBookId = getCurrentBookId();
  if (currentBookId.empty()) {
    return false;
  } else {
    getBookById(currentBookId, book);
    return true;
  }
}

bool Manager::getBookById(const string id, Book& book)
{
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (itr->id == id) {
      book = *itr;
      return true;
    }
  }
  return false;
}

bool Manager::updateBookLastOpenDateById(const string id)
{
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (itr->id == id) {
      char unixdate[12];
      sprintf(unixdate, "%d", (int)time(NULL));
      itr->last = unixdate;
      return true;
    }
  }

  return false;
}

bool Manager::setBookIndex(const string id,
                           const string path,
                           const supportedIndexType type)
{
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (itr->id == id) {
      itr->indexPath = path;
      itr->indexPathAbsolute
          = isRelativePath(path)
                ? computeAbsolutePath(
                      removeLastPathElement(writableLibraryPath, true, false),
                      path)
                : path;
      itr->indexType = type;
      return true;
    }
  }

  return false;
}

bool Manager::setBookPath(const string id, const string path)
{
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if (itr->id == id) {
      itr->path = path;
      itr->pathAbsolute
          = isRelativePath(path)
                ? computeAbsolutePath(
                      removeLastPathElement(writableLibraryPath, true, false),
                      path)
                : path;
      return true;
    }
  }

  return false;
}

void Manager::removeBookPaths()
{
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    itr->path = "";
    itr->pathAbsolute = "";
  }
}

unsigned int Manager::getBookCount(const bool localBooks,
                                   const bool remoteBooks)
{
  unsigned int result = 0;
  std::vector<kiwix::Book>::iterator itr;
  for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
    if ((!itr->path.empty() && localBooks)
        || (itr->path.empty() && remoteBooks)) {
      result++;
    }
  }
  return result;
}

bool Manager::listBooks(const supportedListMode mode,
                        const supportedListSortBy sortBy,
                        const unsigned int maxSize,
                        const string language,
                        const string creator,
                        const string publisher,
                        const string search)
{
  this->bookIdList.clear();
  std::vector<kiwix::Book>::iterator itr;

  /* Sort */
  if (sortBy == TITLE) {
    std::sort(
        library.books.begin(), library.books.end(), kiwix::Book::sortByTitle);
  } else if (sortBy == SIZE) {
    std::sort(
        library.books.begin(), library.books.end(), kiwix::Book::sortBySize);
  } else if (sortBy == DATE) {
    std::sort(
        library.books.begin(), library.books.end(), kiwix::Book::sortByDate);
  } else if (sortBy == CREATOR) {
    std::sort(
        library.books.begin(), library.books.end(), kiwix::Book::sortByCreator);
  } else if (sortBy == PUBLISHER) {
    std::sort(library.books.begin(),
              library.books.end(),
              kiwix::Book::sortByPublisher);
  }

  /* Special sort for LASTOPEN */
  if (mode == LASTOPEN) {
    std::sort(library.books.begin(),
              library.books.end(),
              kiwix::Book::sortByLastOpen);
    for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
      if (!itr->last.empty()) {
        this->bookIdList.push_back(itr->id);
      }
    }
  } else {
    /* Generate the list of book id */
    for (itr = library.books.begin(); itr != library.books.end(); ++itr) {
      bool ok = true;

      if (mode == LOCAL && itr->path.empty()) {
        ok = false;
      }

      if (ok == true && mode == REMOTE
          && (!itr->path.empty() || itr->url.empty())) {
        ok = false;
      }

      if (ok == true && maxSize != 0
          && (unsigned int)atoi(itr->size.c_str()) > maxSize * 1024 * 1024) {
        ok = false;
      }

      if (ok == true && !language.empty()
          && !matchRegex(itr->language, language)) {
        ok = false;
      }

      if (ok == true && !creator.empty() && itr->creator != creator) {
        ok = false;
      }

      if (ok == true && !publisher.empty() && itr->publisher != publisher) {
        ok = false;
      }

      if ((ok == true && !search.empty())
          && !(matchRegex(itr->title, "\\Q" + search + "\\E")
               || matchRegex(itr->description, "\\Q" + search + "\\E")
               || matchRegex(itr->language, "\\Q" + search + "\\E"))) {
        ok = false;
      }

      if (ok == true) {
        this->bookIdList.push_back(itr->id);
      }
    }
  }

  return true;
}


Library Manager::filter(const std::string& search) {
  Library library;

  if (search.empty()) {
    return library;
  }

  for(auto book:this->library.books) {
     if (matchRegex(book.title, "\\Q" + search + "\\E")
         || matchRegex(book.description, "\\Q" + search + "\\E")) {
       library.addBook(book);
     }
  }

  return library;
}

void Manager::checkAndCleanBookPaths(Book& book, const string& libraryPath)
{
  if (!book.path.empty()) {
    if (isRelativePath(book.path)) {
      book.pathAbsolute = computeAbsolutePath(
          removeLastPathElement(libraryPath, true, false), book.path);
    } else {
      book.pathAbsolute = book.path;
      book.path = computeRelativePath(
          removeLastPathElement(libraryPath, true, false), book.pathAbsolute);
    }
  }

  if (!book.indexPath.empty()) {
    if (isRelativePath(book.indexPath)) {
      book.indexPathAbsolute = computeAbsolutePath(
          removeLastPathElement(libraryPath, true, false), book.indexPath);
    } else {
      book.indexPathAbsolute = book.indexPath;
      book.indexPath
          = computeRelativePath(removeLastPathElement(libraryPath, true, false),
                                book.indexPathAbsolute);
    }
  }
}
}
