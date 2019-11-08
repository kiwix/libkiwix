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

#ifndef KIWIX_SEARCH_RENDERER_H
#define KIWIX_SEARCH_RENDERER_H

#include <string>
#include "opds_dumper.h"

namespace kiwix
{

class Searcher;
class NameMapper;
/**
 * The SearcherRenderer class is used to render a search result to a html page.
 */
class SearchRenderer
{
 public:
  /**
   * The default constructor.
   *
   * @param humanReadableName The global zim's humanReadableName.
   *                          Used to generate pagination links.
   */
  SearchRenderer(Searcher* searcher, NameMapper* mapper);

  ~SearchRenderer();

  void setSearchPattern(const std::string& pattern);

  /**
   * Set the search content id.
   */
  void setSearchContent(const std::string& name);

  /**
   * Set protocol prefix.
   */
  void setProtocolPrefix(const std::string& prefix);

  /**
   * Set search protocol prefix.
   */
  void setSearchProtocolPrefix(const std::string& prefix);

  /**
   * Set the search description url for atom feed generation 
   **/
  void setSearchDescriptionUrl(const std::string& url);

  /**
   * Generate the html page with the results of the search.
   */
  std::string getHtml();

  /**
   * Generate the atom feed with the results of the search.
   */
  std::string getAtomFeed();

 protected:
  std::string beautifyInteger(const unsigned int number);
  Searcher* mp_searcher;
  NameMapper* mp_nameMapper;
  std::string searchContent;
  std::string searchPattern;
  std::string protocolPrefix;
  std::string searchProtocolPrefix;
  std::string searchDescriptionUrl;
  unsigned int resultCountPerPage;
  unsigned int estimatedResultCount;
  unsigned int resultStart;
  unsigned int resultEnd;
};


}

#endif
