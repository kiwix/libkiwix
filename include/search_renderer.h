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
#include <zim/search.h>
#include "library.h"

namespace kiwix
{

class NameMapper;
/**
 * The SearcherRenderer class is used to render a search result to a html page.
 */
class SearchRenderer
{
 public:
  /**
   * Construct a SearchRenderer from a SearchResultSet.
   *
   * The constructed version of the SearchRenderer will not introduce
   * the book name for each result. It is better to use the other constructor
   * with a Library pointer to have a better html page.
   *
   * @param srs The `SearchResultSet` to render.
   * @param mapper The `NameMapper` to use to do the rendering.
   * @param start The start offset used for the srs.
   * @param estimatedResultCount The estimatedResultCount of the whole search
   */
  SearchRenderer(zim::SearchResultSet srs, std::shared_ptr<NameMapper> mapper,
                 unsigned int start, unsigned int estimatedResultCount);

  /**
   * Construct a SearchRenderer from a SearchResultSet.
   *
   * @param srs The `SearchResultSet` to render.
   * @param mapper The `NameMapper` to use to do the rendering.
   * @param library The `Library` to use to look up book details for search results.
   * @param start The start offset used for the srs.
   * @param estimatedResultCount The estimatedResultCount of the whole search
   */
  SearchRenderer(zim::SearchResultSet srs, std::shared_ptr<NameMapper> mapper, std::shared_ptr<Library> library,
                 unsigned int start, unsigned int estimatedResultCount);

  ~SearchRenderer();

  /**
   * Set the search pattern used to do the search
   */
  void setSearchPattern(const std::string& pattern);

  /**
   * Set the querystring used to select books
   */
  void setSearchBookQuery(const std::string& bookQuery);

  /**
   * Set protocol prefix.
   */
  void setProtocolPrefix(const std::string& prefix);

  /**
   * Set search protocol prefix.
   */
  void setSearchProtocolPrefix(const std::string& prefix);

  /**
   * set result count per page
   */
  void setPageLength(unsigned int pageLength){
    this->pageLength  = pageLength;
  }

  std::string renderTemplate(const std::string& tmpl_str);

  /**
   * Generate the html page with the resutls of the search.
   */
  std::string getHtml();

    /**
   * Generate the xml page with the resutls of the search.
   */
  std::string getXml();


 protected:
  std::string beautifyInteger(const unsigned int number);
  zim::SearchResultSet m_srs;
  std::shared_ptr<NameMapper> mp_nameMapper;
  std::shared_ptr<Library> mp_library;
  std::string searchBookQuery;
  std::string searchPattern;
  std::string protocolPrefix;
  std::string searchProtocolPrefix;
  unsigned int pageLength;
  unsigned int estimatedResultCount;
  unsigned int resultStart;
};


}

#endif
