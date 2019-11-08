/*
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

#ifndef KIWIX_OPDS_DUMPER_H
#define KIWIX_OPDS_DUMPER_H

#include <time.h>
#include <sstream>
#include <string>

#include <pugixml.hpp>

#include "tools/base64.h"
#include "tools/pathTools.h"
#include "tools/regexTools.h"
#include "library.h"
#include "reader.h"
#include "searcher.h"
#include "name_mapper.h"

using namespace std;

namespace kiwix
{

/**
 * A tool to dump a `Library` into a opds stream.
 *
 */
class OPDSDumper
{
 public:
  OPDSDumper() = default;
  OPDSDumper(Library* library);
  ~OPDSDumper();

  /**
   * Dump the OPDS feed.
   *
   * @param id The id of the library.
   * @return The OPDS feed.
   */
  std::string dumpOPDSFeed(const std::vector<std::string>& bookIds);

  /**
   * Set the id of the opds stream.
   *
   * @param id the id to use.
   */
  void setId(const std::string& id) { this->id = id;}

  /**
   * Set the title oft the opds stream.
   *
   * @param title the title to use.
   */
  void setTitle(const std::string& title) { this->title = title; }

  /**
   * Set the root location used when generating url.
   *
   * @param rootLocation the root location to use.
   */
  void setRootLocation(const std::string& rootLocation) { this->rootLocation = rootLocation; }

  /**
   * Set the search url.
   *
   * @param searchUrl the search url to use.
   */
  void setSearchDescriptionUrl(const std::string& searchDescriptionUrl) { this->searchDescriptionUrl = searchDescriptionUrl; }

  /**
   * Set some informations about the search results.
   *
   * @param totalResult the total number of results of the search.
   * @param startIndex the start index of the result.
   * @param count the number of result of the current set (or page).
   */
  void setOpenSearchInfo(int totalResult, int startIndex, int count);

  /**
   * Set the library to dump.
   *
   * @param library The library to dump.
   */
  void setLibrary(Library* library) { this->library = library; }

  /**
   * Set the search pattern.
   *
   * @param library The library to dump.
   */
  void setSearchPattern(const std::string& searchPattern) { this->searchPattern = searchPattern; }

  /**
   * Set the protocol prefix.
   *
   * @param library The library to dump.
   */
  void setSearchProtocolPrefix(const std::string& searchProtocolPrefix) { this->searchProtocolPrefix = searchProtocolPrefix; }

  /**
   * Set the search pattern.
   *
   * @param library The library to dump.
   */
  void setProtocolPrefix(const std::string& protocolPrefix) { this->protocolPrefix = protocolPrefix; }

  /**
   * Set the result count per page
   **/
  void setResultCountPerPage(const unsigned int resultCountPerPage) { this->resultCountPerPage = resultCountPerPage; }

  /**
   * Set the estimated result count
   **/
  void setEstimatedResultCount(const unsigned int estimatedResultCount) { this->estimatedResultCount = estimatedResultCount; }

  /**
   * Set the result start
   **/
  void setResultStart(const unsigned int resultStart) { this->resultStart = resultStart; }

  /**
   * Dump the OPDS Search result feed
   * @param searcher The searcher object allowing to retrieve results
   **/ 
  std::string dumpSearchResultFeed(Searcher& searcher, NameMapper& nameMapper);

 protected:
  kiwix::Library* library;
  std::string id;
  std::string title;
  std::string date;
  std::string rootLocation;
  std::string searchDescriptionUrl;
  std::string searchPattern;
  std::string searchProtocolPrefix;
  std::string protocolPrefix;
  unsigned int resultCountPerPage;
  unsigned int estimatedResultCount;
  unsigned int resultStart;
  int m_totalResults;
  int m_startIndex;
  int m_count;
  bool m_isSearchResult = false;

 private:
  pugi::xml_node handleBook(Book book, pugi::xml_node root_node);
  pugi::xml_node handleResultEntry(const std::string& uid, Result* result, pugi::xml_node& root_node, NameMapper& nameMapper);
};
}

#endif // KIWIX_OPDS_DUMPER_H
