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
#include "searcher.h"
#include "reader.h"
#include "library.h"
#include "name_mapper.h"

#include "tools/archiveTools.h"

#include <zim/search.h>

#include <mustache.hpp>
#include "kiwixlib-resources.h"
#include "tools/stringTools.h"

namespace kiwix
{

/* Constructor */
SearchRenderer::SearchRenderer(Searcher* searcher, NameMapper* mapper)
    : SearchRenderer(
        searcher->getSearchResultSet(),
        mapper,
        nullptr,
        searcher->getEstimatedResultCount(),
        searcher->getResultStart())
{}

SearchRenderer::SearchRenderer(zim::SearchResultSet srs, NameMapper* mapper,
                      unsigned int start, unsigned int estimatedResultCount)
    : SearchRenderer(srs, mapper, nullptr, start, estimatedResultCount)
{}

SearchRenderer::SearchRenderer(zim::SearchResultSet srs, NameMapper* mapper, Library* library,
                      unsigned int start, unsigned int estimatedResultCount)
    : m_srs(srs),
      mp_nameMapper(mapper),
      mp_library(library),
      protocolPrefix("zim://"),
      searchProtocolPrefix("search://?"),
      estimatedResultCount(estimatedResultCount),
      resultStart(start)
{}

/* Destructor */
SearchRenderer::~SearchRenderer() = default;

void SearchRenderer::setSearchPattern(const std::string& pattern)
{
  this->searchPattern = pattern;
}

void SearchRenderer::setSearchContent(const std::string& name)
{
  this->searchContent = name;
}

void SearchRenderer::setProtocolPrefix(const std::string& prefix)
{
  this->protocolPrefix = prefix;
}

void SearchRenderer::setSearchProtocolPrefix(const std::string& prefix)
{
  this->searchProtocolPrefix = prefix;
}

std::string SearchRenderer::getHtml()
{
  kainjow::mustache::data results{kainjow::mustache::data::type::list};

  for (auto it = m_srs.begin(); it != m_srs.end(); it++) {
    kainjow::mustache::data result;
    result.set("title", it.getTitle());
    result.set("url", it.getPath());
    result.set("snippet", it.getSnippet());
    std::string zim_id(it.getZimId());
    result.set("resultContentId", mp_nameMapper->getNameForId(zim_id));
    if (!mp_library) {
      result.set("bookTitle", kainjow::mustache::data(false));
    } else {
      result.set("bookTitle", mp_library->getBookById(zim_id).getTitle());
    }

    if (it.getWordCount() >= 0) {
      result.set("wordCount", kiwix::beautifyInteger(it.getWordCount()));
    }

    results.push_back(result);
  }

  // pages
  kainjow::mustache::data pages{kainjow::mustache::data::type::list};

  auto resultEnd = 0U;
  auto currentPage = 0U;
  auto pageStart = 0U;
  auto pageEnd = 0U;
  auto lastPageStart = 0U;
  if (pageLength) {
    currentPage = resultStart/pageLength;
    pageStart = currentPage > 4 ? currentPage-4 : 0;
    pageEnd = currentPage + 5;
    if (pageEnd > estimatedResultCount / pageLength) {
      pageEnd = (estimatedResultCount + pageLength - 1) / pageLength;
    }
    if (estimatedResultCount > pageLength) {
      lastPageStart = ((estimatedResultCount-1)/pageLength)*pageLength;
    }
  }

  resultEnd = resultStart+pageLength; //setting result end

  for (unsigned int i = pageStart; i < pageEnd; i++) {
    kainjow::mustache::data page;
    page.set("label", to_string(i + 1));
    page.set("start", to_string(i * pageLength));

    if (i == currentPage) {
      page.set("selected", true);
    }
    pages.push_back(page);
  }

  std::string template_str = RESOURCE::templates::search_result_html;
  kainjow::mustache::mustache tmpl(template_str);

  kainjow::mustache::data allData;
  allData.set("results", results);
  allData.set("pages", pages);
  allData.set("hasResults", estimatedResultCount != 0);
  allData.set("hasPages", pageStart != pageEnd);
  allData.set("count", kiwix::beautifyInteger(estimatedResultCount));
  allData.set("searchPattern", kiwix::encodeDiples(this->searchPattern));
  allData.set("searchPatternEncoded", urlEncode(this->searchPattern));
  allData.set("resultStart", to_string(resultStart + 1));
  allData.set("resultEnd", to_string(min(resultEnd, estimatedResultCount)));
  allData.set("pageLength", to_string(pageLength));
  allData.set("resultLastPageStart", to_string(lastPageStart));
  allData.set("protocolPrefix", this->protocolPrefix);
  allData.set("searchProtocolPrefix", this->searchProtocolPrefix);
  allData.set("contentId", this->searchContent);

  std::stringstream ss;
  tmpl.render(allData, [&ss](const std::string& str) { ss << str; });
  return ss.str();
}

}
