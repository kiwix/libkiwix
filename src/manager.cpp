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
Manager::Manager(Library* library):
  writableLibraryPath(""),
  library(library)
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

    book.setReadOnly(readOnly);
    book.setId(bookNode.attribute("id").value());
    std::string path = bookNode.attribute("path").value();
    if (isRelativePath(path)) {
      path = computeAbsolutePath(
        removeLastPathElement(libraryPath, true, false), path);
    }
    book.setPath(path);
    std::string indexPath = bookNode.attribute("indexPath").value();
    if (isRelativePath(indexPath)) {
      indexPath = computeAbsolutePath(
        removeLastPathElement(libraryPath, true, false), indexPath);
    }
    book.setIndexPath(indexPath);
    book.setIndexType(XAPIAN);
    book.setTitle(bookNode.attribute("title").value());
    book.setName(bookNode.attribute("name").value());
    book.setTags(bookNode.attribute("tags").value());
    book.setDescription(bookNode.attribute("description").value());
    book.setLanguage(bookNode.attribute("language").value());
    book.setDate(bookNode.attribute("date").value());
    book.setCreator(bookNode.attribute("creator").value());
    book.setPublisher(bookNode.attribute("publisher").value());
    book.setUrl(bookNode.attribute("url").value());
    book.setOrigId(bookNode.attribute("origId").value());
    book.setArticleCount(strtoull(bookNode.attribute("articleCount").value(), 0, 0));
    book.setMediaCount(strtoull(bookNode.attribute("mediaCount").value(), 0, 0));
    book.setSize(strtoull(bookNode.attribute("size").value(), 0, 0));
    book.setFavicon(bookNode.attribute("favicon").value());
    book.setFaviconMimeType(bookNode.attribute("faviconMimeType").value());

    /* Update the book properties with the new importer */
    if (libraryVersion.empty()
        || atoi(libraryVersion.c_str()) <= atoi(KIWIX_LIBRARY_VERSION)) {
      ok = false;
      if (!book.path().empty()) {
        ok = this->readBookFromPath(book.path());
      }
    }

    if (ok) {
      library->addBook(book);
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

    book.setReadOnly(false);
    book.setId(entryNode.child("id").child_value());
    book.setTitle(entryNode.child("title").child_value());
    book.setDescription(entryNode.child("summary").child_value());
    book.setLanguage(entryNode.child("language").child_value());
    book.setDate(entryNode.child("updated").child_value());
    book.setCreator(entryNode.child("author").child("name").child_value());
    for(pugi::xml_node linkNode = entryNode.child("link"); linkNode;
        linkNode = linkNode.next_sibling("link")) {
       std::string rel = linkNode.attribute("rel").value();

       if (rel == "http://opds-spec.org/image/thumbnail") {
         auto faviconUrl = urlHost + linkNode.attribute("href").value();
         Downloader downloader;
         auto fileHandle = downloader.download(faviconUrl);
         if (fileHandle.success) {
           auto content = getFileContent(fileHandle.path);
           book.setFavicon(base64_encode((const unsigned char*)content.data(), content.size()));
           book.setFaviconMimeType(linkNode.attribute("type").value());
         } else {
           std::cerr << "Cannot get favicon content from " << faviconUrl << std::endl;
         }

       } else if (rel == "http://opds-spec.org/acquisition/open-access") {
         book.setUrl(linkNode.attribute("href").value());
       }
    }

    /* Update the book properties with the new importer */
    library->addBook(book);
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
      book.setPath(isRelativePath(pathToSave)
                ? computeAbsolutePath(
                      removeLastPathElement(writableLibraryPath, true, false),
                      pathToSave)
                : pathToSave);
    }

    if (!checkMetaData
        || (checkMetaData && !book.title().empty() && !book.language().empty()
            && !book.date().empty())) {
      book.setUrl(url);
      library->addBook(book);
      return book.id();
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
      book->setPath(path);
      book->setId(reader->getId());
      book->setDescription(reader->getDescription());
      book->setLanguage(reader->getLanguage());
      book->setDate(reader->getDate());
      book->setCreator(reader->getCreator());
      book->setPublisher(reader->getPublisher());
      book->setTitle(reader->getTitle());
      book->setName(reader->getName());
      book->setTags(reader->getTags());
      book->setOrigId(reader->getOrigId());
      book->setArticleCount(reader->getArticleCount());
      book->setMediaCount(reader->getMediaCount());
      book->setSize(reader->getFileSize());

      string favicon;
      string faviconMimeType;
      if (reader->getFavicon(favicon, faviconMimeType)) {
        book->setFavicon(base64_encode(
            reinterpret_cast<const unsigned char*>(favicon.c_str()),
            favicon.length()));
        book->setFaviconMimeType(faviconMimeType);
      }
    }

    delete reader;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
}

bool Manager::removeBookById(const string id)
{
  return library.removeBookById(id);
}
bool Manager::setBookIndex(const string id,
                           const string path,
                           const supportedIndexType type)
try {
  auto book = library->getBookById(id);
  book.setIndexPath(isRelativePath(path)
                ? computeAbsolutePath(
                      removeLastPathElement(writableLibraryPath, true, false),
                      path)
                : path);
  book.setIndexType(type);
  return true;
} catch (...) {
  return false;
}

bool Manager::setBookPath(const string id, const string path)
try {
  auto book = library->getBookById(id);
  book.setPath(isRelativePath(path)
     ? computeAbsolutePath(
        removeLastPathElement(writableLibraryPath, true, false),
        path)
     : path);
  return true;
} catch(...) {
  return false;
}

}
