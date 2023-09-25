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


#include <cmath>

#include "search_renderer.h"
#include "library.h"
#include "name_mapper.h"

#include "tools/archiveTools.h"

#include <zim/search.h>

#include <mustache.hpp>
#include "libkiwix-resources.h"
#include "tools/stringTools.h"

namespace kiwix
{

/* Constructor */
SearchRenderer::SearchRenderer(zim::SearchResultSet srs, const NameMapper* mapper,
                      unsigned int start, unsigned int estimatedResultCount)
    : SearchRenderer(srs, mapper, nullptr, start, estimatedResultCount)
{}

SearchRenderer::SearchRenderer(zim::SearchResultSet srs, const NameMapper* mapper, Library* library,
                      unsigned int start, unsigned int estimatedResultCount)
    : m_srs(srs),
      mp_nameMapper(mapper),
      mp_library(library),
      protocolPrefix("zim://"),
      searchProtocolPrefix("search://"),
      estimatedResultCount(estimatedResultCount),
      resultStart(start)
{}

/* Destructor */
SearchRenderer::~SearchRenderer() = default;

void SearchRenderer::setSearchPattern(const std::string& pattern)
{
  searchPattern = pattern;
}

void SearchRenderer::setSearchBookQuery(const std::string& bookQuery)
{
  searchBookQuery = bookQuery;
}

void SearchRenderer::setProtocolPrefix(const std::string& prefix)
{
  this->protocolPrefix = prefix;
}

void SearchRenderer::setSearchProtocolPrefix(const std::string& prefix)
{
  this->searchProtocolPrefix = prefix;
}

std::string extractValueFromQuery(const std::string& query, const std::string& key) {
  const std::string p = key + "=";
  const size_t i = query.find(p);
  if (i == std::string::npos) {
    return "";
  }
  std::string r = query.substr(i + p.size());
  return r.substr(0, r.find("&"));
}

kainjow::mustache::data buildQueryData
(
  const std::string& searchProtocolPrefix,
  const std::string& pattern,
  const std::string& bookQuery
) {
  kainjow::mustache::data query;
  query.set("pattern", kiwix::encodeDiples(pattern));
  std::ostringstream ss;
  ss << searchProtocolPrefix << "?pattern=" << urlEncode(pattern);
  ss << "&" << bookQuery;
  query.set("unpaginatedQuery", ss.str());
  auto lang = extractValueFromQuery(bookQuery, "books.filter.lang");
  if(!lang.empty()) {
    query.set("lang", lang);
  }
  return query;
}

kainjow::mustache::data buildPagination(
  unsigned int pageLength,
  unsigned int resultsCount,
  unsigned int resultsStart
)
{
  assert(pageLength!=0);
  kainjow::mustache::data pagination;
  kainjow::mustache::data pages{kainjow::mustache::data::type::list};

  if (resultsCount == 0) {
    // Easy case
    pagination.set("itemsPerPage", to_string(pageLength));
    pagination.set("hasPages", false);
    pagination.set("pages", pages);
    return pagination;
  }

  // First we want to display pages starting at a multiple of `pageLength`
  // so, let's calculate the start index of the current page.
  auto currentPage = resultsStart/pageLength;
  auto lastPage = ((resultsCount-1)/pageLength);
  auto lastPageStart = lastPage*pageLength;
  auto nbPages = lastPage + 1;

  auto firstPageGenerated = currentPage > 4 ? currentPage-4 : 0;
  auto lastPageGenerated = std::min(currentPage+4, lastPage);

  if (nbPages != 1) {
    if (firstPageGenerated!=0) {
      kainjow::mustache::data page;
      page.set("label", "◀");
      page.set("start", to_string(0));
      page.set("current", false);
      pages.push_back(page);
    }

    for (auto i=firstPageGenerated; i<=lastPageGenerated; i++) {
      kainjow::mustache::data page;
      page.set("label", to_string(i+1));
      page.set("start", to_string(i*pageLength));
      page.set("current", bool(i == currentPage));
      pages.push_back(page);
    }

    if (lastPageGenerated!=lastPage) {
      kainjow::mustache::data page;
      page.set("label", "▶");
      page.set("start", to_string(lastPageStart));
      page.set("current", false);
      pages.push_back(page);
    }
  }

  pagination.set("itemsPerPage", to_string(pageLength));
  pagination.set("hasPages", firstPageGenerated < lastPageGenerated);
  pagination.set("pages", pages);
  return pagination;
}

std::string SearchRenderer::renderTemplate(const std::string& tmpl_str)
{
  const std::string absPathPrefix = protocolPrefix;
  // Build the results list
  kainjow::mustache::data items{kainjow::mustache::data::type::list};
  for (auto it = m_srs.begin(); it != m_srs.end(); it++) {
    kainjow::mustache::data result;
    const std::string zim_id(it.getZimId());
    const auto path = mp_nameMapper->getNameForId(zim_id) + "/" + it.getPath();
    result.set("title", it.getTitle());
    result.set("absolutePath", absPathPrefix + urlEncode(path));
    result.set("snippet", it.getSnippet());
    if (mp_library) {
      result.set("bookTitle", mp_library->getBookById(zim_id).getTitle());
    }
    if (it.getWordCount() >= 0) {
      result.set("wordCount", kiwix::beautifyInteger(it.getWordCount()));
    }

    items.push_back(result);
  }
  kainjow::mustache::data results;
  results.set("items", items);
  results.set("count", kiwix::beautifyInteger(estimatedResultCount));
  results.set("hasResults", estimatedResultCount != 0);
  results.set("start", kiwix::beautifyInteger(resultStart));
  results.set("end", kiwix::beautifyInteger(std::min(resultStart+pageLength-1, estimatedResultCount)));

  // pagination
  auto pagination = buildPagination(
    pageLength,
    estimatedResultCount,
    resultStart
  );

  kainjow::mustache::data query = buildQueryData(
    searchProtocolPrefix,
    searchPattern,
    searchBookQuery
  );


  kainjow::mustache::data allData;
  allData.set("searchProtocolPrefix", searchProtocolPrefix);
  allData.set("results", results);
  allData.set("pagination", pagination);
  allData.set("query", query);

  kainjow::mustache::mustache tmpl(tmpl_str);

  std::stringstream ss;
  tmpl.render(allData, [&ss](const std::string& str) { ss << str; });
  if (!tmpl.is_valid()) {
    throw std::runtime_error("Error while rendering search results: " + tmpl.error_message());
  }
  return ss.str();
}

std::string SearchRenderer::getHtml()
{
  return renderTemplate(RESOURCE::templates::search_result_html);
}

std::string SearchRenderer::getXml()
{
  return renderTemplate(RESOURCE::templates::search_result_xml);
}


}
