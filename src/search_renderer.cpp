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

#include <zim/search.h>

#include <mustache.hpp>
#include "kiwixlib-resources.h"


namespace kiwix
{

/* Constructor */
SearchRenderer::SearchRenderer(Searcher* searcher, NameMapper* mapper)
    : mp_searcher(searcher),
      mp_nameMapper(mapper),
      protocolPrefix("zim://"),
      searchProtocolPrefix("search://?")
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

  mp_searcher->restart_search();
  Result* p_result = NULL;
  while ((p_result = mp_searcher->getNextResult())) {
    kainjow::mustache::data result;
    result.set("title", p_result->get_title());
    result.set("url", p_result->get_url());
    result.set("snippet", p_result->get_snippet());
    result.set("resultContentId", mp_nameMapper->getNameForId(p_result->get_zimId()));

    if (p_result->get_wordCount() >= 0) {
      result.set("wordCount", kiwix::beautifyInteger(p_result->get_wordCount()));
    }

    results.push_back(result);
    delete p_result;
  }

  // pages
  kainjow::mustache::data pages{kainjow::mustache::data::type::list};

  auto resultStart = mp_searcher->getResultStart();
  auto resultEnd = 0U;
  auto estimatedResultCount = mp_searcher->getEstimatedResultCount();
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