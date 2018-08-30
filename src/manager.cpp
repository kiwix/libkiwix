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
#include "downloader.h"

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



bool Manager::parseOpdsDom(const pugi::xml_document& doc, const std::string& urlHost)
{
  pugi::xml_node libraryNode = doc.child("feed");

  for (pugi::xml_node entryNode = libraryNode.child("entry"); entryNode;
       entryNode = entryNode.next_sibling("entry")) {
    kiwix::Book book;

    book.readOnly = false;
    book.id = entryNode.child("id").child_value();
    book.title = entryNode.child("title").child_value();
    book.description = entryNode.child("summary").child_value();
    book.language = entryNode.child("language").child_value();
    book.date = entryNode.child("updated").child_value();
    book.creator = entryNode.child("author").child("name").child_value();
    for(pugi::xml_node linkNode = entryNode.child("link"); linkNode;
        linkNode = linkNode.next_sibling("link")) {
       std::string rel = linkNode.attribute("rel").value();

       if (rel == "http://opds-spec.org/image/thumbnail") {
         auto faviconUrl = urlHost + linkNode.attribute("href").value();
         Downloader downloader;
         auto fileHandle = downloader.download(faviconUrl);
         if (fileHandle.success) {
           auto content = getFileContent(fileHandle.path);
           book.favicon = base64_encode((const unsigned char*)content.data(), content.size());
           book.faviconMimeType = linkNode.attribute("type").value();
         } else {
           std::cerr << "Cannot get favicon content from " << faviconUrl << std::endl;
         }

       } else if (rel == "http://opds-spec.org/acquisition/open-access") {
         book.url = linkNode.attribute("href").value();
       }
    }

    /* Update the book properties with the new importer */
    library.addBook(book);
  }

  return true;
}



bool Manager::readOpds(const string& content, const std::string& urlHost)
{
  pugi::xml_document doc;
  pugi::xml_parse_result result
      = doc.load_buffer_inplace((void*)content.data(), content.size());

  if (result) {
    this->parseOpdsDom(doc, urlHost);
    return true;
  }

  return false;
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
  return library.removeBookById(id);
}

bool Manager::updateBookLastOpenDateById(const string id)
try {
  auto book = library.getBookById(id);
  char unixdate[12];
  sprintf(unixdate, "%d", (int)time(NULL));
  book.last = unixdate;
  return true;
} catch(...) {
  return false;
}

bool Manager::setBookIndex(const string id,
                           const string path,
                           const supportedIndexType type)
try {
  auto book = library.getBookById(id);
  book.indexPath = path;
  book.indexPathAbsolute = isRelativePath(path)
                ? computeAbsolutePath(
                      removeLastPathElement(writableLibraryPath, true, false),
                      path)
                : path;
  book.indexType = type;
  return true;
} catch (...) {
  return false;
}

bool Manager::setBookPath(const string id, const string path)
try {
  auto book = library.getBookById(id);
  book.path = path;
  book.pathAbsolute = isRelativePath(path)
     ? computeAbsolutePath(
        removeLastPathElement(writableLibraryPath, true, false),
        path)
     : path;
  return true;
} catch(...) {
  return false;
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
