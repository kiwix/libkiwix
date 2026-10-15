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

#include "book.h"
#include "library.h"

#include <string>
#include <vector>
#include <memory>

namespace pugi {
class xml_document;
}

namespace kiwix
{

class LibraryManipulator
{
 public: // functions
  explicit LibraryManipulator(LibraryPtr library);
  virtual ~LibraryManipulator();

  LibraryPtr getLibrary() const { return library; }

  bool addBookToLibrary(const Book& book);
  void addBookmarkToLibrary(const Bookmark& bookmark);
  uint32_t removeBooksNotUpdatedSince(Library::Revision rev);

 protected: // overrides
  virtual void bookWasAddedToLibrary(const Book& book);
  virtual void bookmarkWasAddedToLibrary(const Bookmark& bookmark);
  virtual void booksWereRemovedFromLibrary();

 private: // data
  LibraryPtr library;
};

/**
 * A tool to manage a `Library`.
 */
class Manager
{
 public: // types
  typedef std::vector<std::string> Paths;

 public: // functions
  explicit Manager(LibraryManipulator manipulator);
  explicit Manager(LibraryPtr library);

  /**
   * Read an OPDS library file and add the books in it to the library.
   *
   * The book metadata stored in the file is used as is; the ZIM files are
   * not opened. A relative path in a local acquisition link is resolved
   * against the directory containing the library file.
   *
   * The books are added as writable (not read-only), so a later
   * Library::addBook() of a book with the same id updates them.
   *
   * `path` is recorded as the writable library path even if the file
   * doesn't exist or can't be read, so that the library can later be
   * written there. It is also used to resolve relative ZIM paths passed to
   * addBookFromPathAndGetId().
   *
   * @param path The (utf8) path to the library file.
   * @return True if the file exists and is a valid OPDS feed.
   */
  bool readFile(const std::string& path);

  /**
   * Sync the contents of the library with one or more OPDS library files.
   *
   * Each file is read with readFile(), so the metadata of the library files
   * is trusted unconditionally. Any books not present in the input library
   * files are removed from the library.
   *
   * @param paths The (utf8) paths to the OPDS library files.
   * @throw std::runtime_error if one of the files can't be loaded (missing,
   *        unreadable or not an OPDS feed).
   */
  void reload(const Paths& paths);

  /**
   * Load a library content stored in a OPDS stream or a local library file.
   *
   * @param content The content of the OPDS stream.
   * @param contentOriginUri Where content was read from: either the URL it
   *        was fetched from (used to resolve relative acquisition/thumbnail
   *        links) or the local filesystem path it was read from (used to
   *        resolve a relative local-path acquisition link). See
   *        kiwix::resolveContentOrigin() for the exact splitting rules. It is
   *        assumed that URL links come in the form of an absolute path
   *        component.
   * @param readOnly Set if the library path could be overwritten later with
   *                 updated content.
   * @return True if the content has been properly parsed.
   */
  bool readOpds(const std::string& content,
                const std::string& contentOriginUri,
                bool readOnly);

  /**
   * Load a library content stored in a OPDS stream or a local library file.
   *
   * A simple wrapper around the three-parameter readOpds() above, kept for
   * backward compatibility. Equivalent to calling
   * readOpds(content, urlHost, false).
   *
   * @param content The content of the OPDS stream.
   * @param urlHost The host used to resolve relative acquisition/thumbnail
   *                links. It is assumed that those links come in the form of an
   *                absolute path component, so the full URL is obtained by
   *                prepending the urlHost string to the href value of the link.
   * @return True if the content has been properly parsed.
   */
  bool readOpds(const std::string& content, const std::string& urlHost);

  /**
   * Load a bookmark file.
   *
   * @param path The path of the file to read.
   * @return True if the content has been properly parsed.
   */
  bool readBookmarkFile(const std::string& path);

  /**
   * Add a book to the library.
   *
   * @param pathToOpen The path to the zim file to add.
   * @param pathToSave The path to store in the library in place of pathToOpen.
   * @param url        The url of the book to store in the library.
   * @param checMetaData Tell if we check metadata before adding book to the
   *                     library.
   * @return The id of the book if the book has been added to the library.
   *         Else, an empty string.
   */
  std::string addBookFromPathAndGetId(const std::string& pathToOpen,
                                 const std::string& pathToSave = "",
                                 const std::string& url = "",
                                 const bool checkMetaData = false);

  /**
   * Add a book to the library, recording an acquisition link for each
   * non-empty URL in `urls`.
   *
   * Unlike the single-URL overload above, this one takes the URLs grouped
   * in a `Book::AcquisitionLinks` array (indexed by `Book::AcquisitionLinkKind`)
   * instead of a single positional string, so more than one acquisition
   * link (e.g. the ZIM itself, a Metalink, a torrent) can be recorded at
   * once.
   *
   * @param pathToOpen The path to the zim file to add.
   * @param pathToSave The path to store in the library in place of pathToOpen.
   * @param urls       The acquisition URLs of the book to store in the
   *                   library, indexed by kind. Empty entries are not
   *                   recorded.
   * @param checkMetaData Tell if we check metadata before adding book to the
   *                      library.
   * @return The id of the book if the book has been added to the library.
   *         Else, an empty string.
   */
  std::string addBookFromPathAndGetId(const std::string& pathToOpen,
                                 const std::string& pathToSave,
                                 const Book::AcquisitionLinks& urls,
                                 const bool checkMetaData = false);

  /**
   * Add a book to the library.
   *
   * @param pathToOpen The path to the zim file to add.
   * @param pathToSave The path to store in the library in place of pathToOpen.
   * @param url        The url of the book to store in the library.
   * @param checMetaData Tell if we check metadata before adding book to the
   *                     library.
   * @return True if the book has been added to the library.
   */

  bool addBookFromPath(const std::string& pathToOpen,
                       const std::string& pathToSave = "",
                       const std::string& url = "",
                       const bool checkMetaData = false);

  /**
   * Add all books from the directory tree into the library.
   *
   * @param path          The path of the directory to scan.
   * @param verboseFlag   Verbose logs flag.
   */
  void addBooksFromDirectory(const std::string& path,
                             const bool verboseFlag = false);

  std::string writableLibraryPath;

 protected:
  kiwix::LibraryManipulator manipulator;

  bool readBookFromPath(const std::string& path, Book* book);
  bool parseOpdsDom(const pugi::xml_document& doc,
                    const std::string& urlHost,
                    const std::string& baseDir,
                    bool readOnly);
};
}  // namespace kiwix

#endif
