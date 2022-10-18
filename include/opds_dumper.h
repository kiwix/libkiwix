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
#include <vector>

#include <pugixml.hpp>

#include "server.h"

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
  OPDSDumper(Server::Configuration configuration);
  ~OPDSDumper();

  /**
   * Dump the OPDS feed.
   *
   * @param bookIds the ids of the books to include in the feed
   * @param query the query used to obtain the list of book ids
   * @return The OPDS feed.
   */
  std::string dumpOPDSFeed(const std::vector<std::string>& bookIds, const std::string& query) const;

  /**
   * Dump the OPDS feed.
   *
   * @param bookIds the ids of the books to include in the feed
   * @param query the query used to obtain the list of book ids
   * @param partial whether the feed should include partial or complete entries
   * @return The OPDS feed.
   */
  std::string dumpOPDSFeedV2(const std::vector<std::string>& bookIds, const std::string& query, bool partial) const;

  /**
   * Dump the OPDS complete entry document.
   *
   * @param bookId the id of the book
   * @return The OPDS complete entry document.
   */
  std::string dumpOPDSCompleteEntry(const std::string& bookId) const;

  /**
   * Dump the categories OPDS feed.
   *
   * @return The OPDS feed.
   */
  std::string categoriesOPDSFeed() const;

  /**
   * Dump the languages OPDS feed.
   *
   * @return The OPDS feed.
   */
  std::string languagesOPDSFeed() const;

  /**
   * Set the id of the library.
   *
   * @param id the id to use.
   */
  void setLibraryId(const std::string& id) { this->libraryId = id;}

  /**
   * Set some informations about the search results.
   *
   * @param totalResult the total number of results of the search.
   * @param startIndex the start index of the result.
   * @param count the number of result of the current set (or page).
   */
  void setOpenSearchInfo(int totalResult, int startIndex, int count);

 protected:
  Server::Configuration m_configuration;
  std::string libraryId;
  int m_totalResults;
  int m_startIndex;
  int m_count;
};
}

#endif // KIWIX_OPDS_DUMPER_H
