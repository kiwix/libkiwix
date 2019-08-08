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

#include "searcher.h"
#include "reader.h"

#include <zim/search.h>

#include <mustache.hpp>
#include "kiwixlib-resources.h"

#define MAX_SEARCH_LEN 140

namespace kiwix
{
class _Result : public Result
{
 public:
  _Result(zim::Search::iterator& iterator);
  virtual ~_Result(){};

  virtual std::string get_url();
  virtual std::string get_title();
  virtual int get_score();
  virtual std::string get_snippet();
  virtual std::string get_content();
  virtual int get_wordCount();
  virtual int get_size();
  virtual int get_readerIndex();

 private:
  zim::Search::iterator iterator;
};

struct SearcherInternal {
  const zim::Search* _search;
  zim::Search::iterator current_iterator;

  SearcherInternal() : _search(NULL) {}
  ~SearcherInternal()
  {
    if (_search != NULL) {
      delete _search;
    }
  }
};

/* Constructor */
Searcher::Searcher(const std::string& humanReadableName)
    : internal(new SearcherInternal()),
      searchPattern(""),
      protocolPrefix("zim://"),
      searchProtocolPrefix("search://?"),
      resultCountPerPage(0),
      estimatedResultCount(0),
      resultStart(0),
      resultEnd(0),
      contentHumanReadableId(humanReadableName)
{
  loadICUExternalTables();
}

/* Destructor */
Searcher::~Searcher()
{
  delete internal;
}

bool Searcher::add_reader(Reader* reader, const std::string& humanReadableName)
{
  if (!reader->hasFulltextIndex()) {
      return false;
  }
  this->readers.push_back(reader);
  this->humanReaderNames.push_back(humanReadableName);
  return true;
}

/* Search strings in the database */
void Searcher::search(std::string& search,
                      unsigned int resultStart,
                      unsigned int resultEnd,
                      const bool verbose)
{
  this->reset();

  if (verbose == true) {
    cout << "Performing query `" << search << "'" << endl;
  }

  /* Try to find results */
  if (resultStart != resultEnd) {
    /* Perform the search */
    this->searchPattern = search;
    this->resultStart = resultStart;
    this->resultEnd = resultEnd;
    string unaccentedSearch = removeAccents(search);
    std::vector<const zim::File*> zims;
    for (auto current = this->readers.begin(); current != this->readers.end();
         current++) {
      if ( (*current)->hasFulltextIndex() ) {
          zims.push_back((*current)->getZimFileHandler());
      }
    }
    zim::Search* search = new zim::Search(zims);
    search->set_verbose(verbose);
    search->set_query(unaccentedSearch);
    search->set_range(resultStart, resultEnd);
    internal->_search = search;
    internal->current_iterator = internal->_search->begin();
    this->estimatedResultCount = internal->_search->get_matches_estimated();
  }

  return;
}


void Searcher::geo_search(float latitude, float longitude, float distance,
                          unsigned int resultStart,
                          unsigned int resultEnd,
                          const bool verbose)
{
  this->reset();

  if (verbose == true) {
    cout << "Performing geo query `" << distance << "&(" << latitude << ";" << longitude << ")'" << endl;
  }

  /* Try to find results */
  if (resultStart == resultEnd) {
    return;
  }

  /* Perform the search */
  std::ostringstream oss;
  oss << "Articles located less than " << distance << " meters of " << latitude << ";" << longitude;
  this->searchPattern = oss.str();
  this->resultStart = resultStart;
  this->resultEnd = resultEnd;

  std::vector<const zim::File*> zims;
  for (auto current = this->readers.begin(); current != this->readers.end();
       current++) {
    zims.push_back((*current)->getZimFileHandler());
  }
  zim::Search* search = new zim::Search(zims);
  search->set_verbose(verbose);
  search->set_query("");
  search->set_georange(latitude, longitude, distance);
  search->set_range(resultStart, resultEnd);
  internal->_search = search;
  internal->current_iterator = internal->_search->begin();
  this->estimatedResultCount = internal->_search->get_matches_estimated();
}


void Searcher::restart_search()
{
  if (internal->_search) {
    internal->current_iterator = internal->_search->begin();
  }
}

Result* Searcher::getNextResult()
{
  if (internal->_search &&
             internal->current_iterator != internal->_search->end()) {
    Result* result = new _Result(internal->current_iterator);
    internal->current_iterator++;
    return result;
  }
  return NULL;
}

/* Reset the results */
void Searcher::reset()
{
  this->estimatedResultCount = 0;
  this->searchPattern = "";
  return;
}

void Searcher::suggestions(std::string& searchPattern, const bool verbose)
{
  this->reset();

  if (verbose == true) {
    cout << "Performing suggestion query `" << searchPattern << "`" << endl;
  }

  this->searchPattern = searchPattern;
  this->resultStart = 0;
  this->resultEnd = 10;
  string unaccentedSearch = removeAccents(searchPattern);

  std::vector<const zim::File*> zims;
  for (auto current = this->readers.begin(); current != this->readers.end();
       current++) {
    zims.push_back((*current)->getZimFileHandler());
  }
  zim::Search* search = new zim::Search(zims);
  search->set_verbose(verbose);
  search->set_query(unaccentedSearch);
  search->set_range(resultStart, resultEnd);
  search->set_suggestion_mode(true);
  internal->_search = search;
  internal->current_iterator = internal->_search->begin();
  this->estimatedResultCount = internal->_search->get_matches_estimated();
}

/* Return the result count estimation */
unsigned int Searcher::getEstimatedResultCount()
{
  return this->estimatedResultCount;
}

bool Searcher::setProtocolPrefix(const std::string prefix)
{
  this->protocolPrefix = prefix;
  return true;
}

bool Searcher::setSearchProtocolPrefix(const std::string prefix)
{
  this->searchProtocolPrefix = prefix;
  return true;
}

_Result::_Result(zim::Search::iterator& iterator)
    : iterator(iterator)
{
}

std::string _Result::get_url()
{
  return iterator.get_url();
}
std::string _Result::get_title()
{
  return iterator.get_title();
}
int _Result::get_score()
{
  return iterator.get_score();
}
std::string _Result::get_snippet()
{
  return iterator.get_snippet();
}
std::string _Result::get_content()
{
  if (iterator->good()) {
    return iterator->getData();
  }
  return "";
}
int _Result::get_size()
{
  return iterator.get_size();
}
int _Result::get_wordCount()
{
  return iterator.get_wordCount();
}
int _Result::get_readerIndex()
{
  return iterator.get_fileIndex();
}

string Searcher::getHtml()
{
  kainjow::mustache::data results{kainjow::mustache::data::type::list};

  this->restart_search();
  Result* p_result = NULL;
  while ((p_result = this->getNextResult())) {
    kainjow::mustache::data result;
    result.set("title", p_result->get_title());
    result.set("url", p_result->get_url());
    result.set("snippet", p_result->get_snippet());
    result.set("resultContentId", humanReaderNames[p_result->get_readerIndex()]);

    if (p_result->get_wordCount() >= 0) {
      result.set("wordCount", kiwix::beautifyInteger(p_result->get_wordCount()));
    }

    results.push_back(result);
    delete p_result;
  }

  // pages
  kainjow::mustache::data pages{kainjow::mustache::data::type::list};

  unsigned int pageStart
      = this->resultStart / this->resultCountPerPage >= 5
            ? this->resultStart / this->resultCountPerPage - 4
            : 0;
  unsigned int pageCount
      = this->estimatedResultCount / this->resultCountPerPage + 1 - pageStart;

  if (pageCount > 10) {
    pageCount = 10;
  } else if (pageCount == 1) {
    pageCount = 0;
  }

  for (unsigned int i = pageStart; i < pageStart + pageCount; i++) {
    kainjow::mustache::data page;
    page.set("label", to_string(i + 1));
    page.set("start", to_string(i * this->resultCountPerPage));
    page.set("end", to_string((i + 1) * this->resultCountPerPage));

    if (i * this->resultCountPerPage == this->resultStart) {
      page.set("selected", true);
    }
    pages.push_back(page);
  }

  std::string template_str = RESOURCE::search_result_tmpl;
  kainjow::mustache::mustache tmpl(template_str);

  kainjow::mustache::data allData;
  allData.set("results", results);
  allData.set("pages", pages);
  allData.set("hasResult", this->estimatedResultCount != 0);
  allData.set("count", kiwix::beautifyInteger(this->estimatedResultCount));
  allData.set("searchPattern", kiwix::encodeDiples(this->searchPattern));
  allData.set("searchPatternEncoded", urlEncode(this->searchPattern));
  allData.set("resultStart", to_string(this->resultStart + 1));
  allData.set("resultEnd", to_string(min(this->resultEnd, this->estimatedResultCount)));
  allData.set("resultRange", to_string(this->resultCountPerPage));
  allData.set("resultLastPageStart", to_string(this->estimatedResultCount > this->resultCountPerPage
             ? round(this->estimatedResultCount / this->resultCountPerPage) * this->resultCountPerPage
             : 0));
  allData.set("lastResult", to_string(this->estimatedResultCount));
  allData.set("protocolPrefix", this->protocolPrefix);
  allData.set("searchProtocolPrefix", this->searchProtocolPrefix);
  allData.set("contentId", this->contentHumanReadableId);

  std::stringstream ss;
  tmpl.render(allData, [&ss](const std::string& str) { ss << str; });
  return ss.str();
}

}
