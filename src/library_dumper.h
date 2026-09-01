/*
 * Copyright 2023 Nikhil Tanwar <2002nikhiltanwar@gmail.com>
 * Copyright 2017 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_LIBRARY_DUMPER_H
#define KIWIX_LIBRARY_DUMPER_H

#include <string>


#include "library.h"
#include "name_mapper.h"
#include <mustache.hpp>

namespace kiwix
{

class Book;

/**
 * Render the full OPDS entry XML for a book.
 *
 * @param book The book to render the OPDS entry for.
 * @param rootLocation The root URL/path the catalog is served from, used to
 *                 build absolute links within the entry.
 * @param contentAccessUrl The URL (or URL prefix) used to access the book's
 *                 content itself (as opposed to the catalog entry).
 * @param contentId The identifier of the book's content, used when building
 *                 content access links.
 * @param localPath If non-empty, rendered as an additional rel="http://opds-
 *                 spec.org/acquisition/open-access" link carrying the book's
 *                 local file path (as opposed to the book's remote url, if
 *                 any, which is rendered as its own such link). Only meant
 *                 for local/offline dumps - leave empty for the live HTTP
 *                 catalog, which must not leak server-side filesystem paths
 *                 to remote clients.
 * @param isLiveCatalog Whether this entry is rendered for the live HTTP
 *                 catalog (OPDSDumper) as opposed to a local/offline file
 *                 dump (Library::dumpOpds). The live catalog can serve any
 *                 illustration - embedded or remote - through its own
 *                 /catalog/v2/illustration endpoint, but an offline dump has
 *                 no server behind that endpoint, so only illustrations
 *                 backed by a real external url can produce a working
 *                 thumbnail link there.
 */
std::string fullEntryOpds(const Book& book,
                          const std::string& rootLocation,
                          const std::string& contentAccessUrl,
                          const std::string& contentId,
                          const std::string& localPath = "",
                          bool isLiveCatalog = true);

/**
 * A base class to dump Library in various formats.
 *
 */
class LibraryDumper
{
 public:
  LibraryDumper(const Library* library, const NameMapper* NameMapper);
  ~LibraryDumper();

  void setLibraryId(const std::string& id) { this->libraryId = id;}

  /**
   * Set the root location used when generating url.
   *
   * @param rootLocation the root location to use.
   */
  void setRootLocation(const std::string& rootLocation) { this->rootLocation = rootLocation; }

  /**
   * Set the URL for accessing book content
   *
   * @param url the URL of the /content endpoint of the content server
   */
  void setContentAccessUrl(const std::string& url) { this->contentAccessUrl = url; }

  /**
   * Sets user default language
   *
   * @param userLang the user language to be set
   */
  void setUserLanguage(std::string userLang) { this->m_userLang = userLang; }

  /**
   * Get the data of categories
   */
  kainjow::mustache::list getCategoryData() const;

  /**
   * Get the data of languages
   */
  kainjow::mustache::list getLanguageData() const;

 protected:
  const kiwix::Library* const library;
  const kiwix::NameMapper* const nameMapper;
  std::string libraryId;
  std::string rootLocation;
  std::string contentAccessUrl;
  std::string m_userLang;
};
}

#endif // KIWIX_LIBRARY_DUMPER_H
