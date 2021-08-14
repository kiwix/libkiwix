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



#include "searcher.h"
#include "reader.h"

#include <zim/search.h>
#include <zim/suggestion.h>

#include <mustache.hpp>
#include <cmath>
#include "tools/stringTools.h"
#include "kiwixlib-resources.h"

#define MAX_SEARCH_LEN 140

namespace kiwix
{
class _Result : public Result
{
 public:
  _Result(zim::SearchResultSet::iterator iterator);
  _Result(SuggestionItem suggestionItem);
  virtual ~_Result(){};

  virtual std::string get_url();
  virtual std::string get_title();
  virtual int get_score();
  virtual std::string get_snippet();
  virtual std::string get_content();
  virtual int get_wordCount();
  virtual int get_size();
  virtual std::string get_zimId();

 private:
  zim::SearchResultSet::iterator iterator;
  SuggestionItem suggestionItem;
  bool isSuggestion;
};

struct SearcherInternal : zim::SearchResultSet {
  explicit SearcherInternal(const zim::SearchResultSet& srs)
    : zim::SearchResultSet(srs)
    , current_iterator(srs.begin())
  {
  }

  zim::SearchResultSet::iterator current_iterator;
};

struct SuggestionInternal : zim::SuggestionResultSet {
  explicit SuggestionInternal(const zim::SuggestionResultSet& srs)
    : zim::SuggestionResultSet(srs),
      currentIterator(srs.begin()) {}

  zim::SuggestionResultSet::iterator currentIterator;
};

/* Constructor */
Searcher::Searcher()
    : searchPattern(""),
      estimatedResultCount(0),
      resultStart(0),
      maxResultCount(0)
{
  loadICUExternalTables();
}

/* Destructor */
Searcher::~Searcher()
{
}

bool Searcher::add_reader(Reader* reader)
{
  if (!reader->hasFulltextIndex()) {
      return false;
  }
  this->readers.push_back(reader);
  return true;
}


Reader* Searcher::get_reader(int readerIndex)
{
  return readers.at(readerIndex);
}

/* Search strings in the database */
void Searcher::search(const std::string& search,
                      unsigned int resultStart,
                      unsigned int maxResultCount,
                      const bool verbose)
{
  this->reset();

  if (verbose == true) {
    cout << "Performing query `" << search << "'" << endl;
  }

  this->searchPattern = search;
  this->resultStart = resultStart;
  this->maxResultCount = maxResultCount;
  /* Try to find results */
  if (maxResultCount != 0) {
    /* Perform the search */
    string unaccentedSearch = removeAccents(search);
    std::vector<zim::Archive> archives;
    for (auto current = this->readers.begin(); current != this->readers.end();
         current++) {
      if ( (*current)->hasFulltextIndex() ) {
          archives.push_back(*(*current)->getZimArchive());
      }
    }
    zim::Searcher searcher(archives);
    searcher.setVerbose(verbose);
    zim::Query query;
    query.setQuery(unaccentedSearch);
    zim::Search search = searcher.search(query);
    internal.reset(new SearcherInternal(search.getResults(resultStart, maxResultCount)));
    this->estimatedResultCount = search.getEstimatedMatches();
  }

  return;
}


void Searcher::geo_search(float latitude, float longitude, float distance,
                          unsigned int resultStart,
                          unsigned int maxResultCount,
                          const bool verbose)
{
  this->reset();

  if (verbose == true) {
    cout << "Performing geo query `" << distance << "&(" << latitude << ";" << longitude << ")'" << endl;
  }

  /* Perform the search */
  std::ostringstream oss;
  oss << "Articles located less than " << distance << " meters of " << latitude << ";" << longitude;
  this->searchPattern = oss.str();
  this->resultStart = resultStart;
  this->maxResultCount = maxResultCount;

  /* Try to find results */
  if (maxResultCount == 0) {
    return;
  }

  std::vector<zim::Archive> archives;
  for (auto current = this->readers.begin(); current != this->readers.end();
       current++) {
    archives.push_back(*(*current)->getZimArchive());
  }
  zim::Searcher searcher(archives);
  searcher.setVerbose(verbose);
  zim::Query query;
  query.setQuery("");
  query.setGeorange(latitude, longitude, distance);
  zim::Search search = searcher.search(query);
  internal.reset(new SearcherInternal(search.getResults(resultStart, maxResultCount)));
  this->estimatedResultCount = search.getEstimatedMatches();
}


void Searcher::restart_search()
{
  if (internal.get()) {
    internal->current_iterator = internal->begin();
  }
}

Result* Searcher::getNextResult()
{
  if (internal.get() && internal->current_iterator != internal->end()) {
    Result* result = new _Result(internal->current_iterator);
    internal->current_iterator++;
    return result;
  } else if (suggestionInternal.get() &&
             suggestionInternal->currentIterator != suggestionInternal->end()) {
    SuggestionItem item(
                        suggestionInternal->currentIterator->getTitle(),
                        normalize(suggestionInternal->currentIterator->getTitle()),
                        suggestionInternal->currentIterator->getPath(),
                        suggestionInternal->currentIterator->getSnippet()
                      );
    Result* result = new _Result(item);
    suggestionInternal->currentIterator++;
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
  this->maxResultCount = 10;
  string unaccentedSearch = removeAccents(searchPattern);

  // Multizim suggestion is not supported as of now! taking only one archive
  zim::Archive archive = *(*this->readers.begin())->getZimArchive();
  zim::SuggestionSearcher searcher(archive);
  searcher.setVerbose(verbose);
  zim::SuggestionSearch search = searcher.suggest(searchPattern);
  suggestionInternal.reset(new SuggestionInternal(search.getResults(resultStart, maxResultCount)));
  this->estimatedResultCount = search.getEstimatedMatches();
}

/* Return the result count estimation */
unsigned int Searcher::getEstimatedResultCount()
{
  return this->estimatedResultCount;
}

zim::SearchResultSet Searcher::getSearchResultSet()
{
  return *(this->internal);
}

_Result::_Result(zim::SearchResultSet::iterator iterator)
    : iterator(iterator),
      suggestionItem("", "", ""),
      isSuggestion(false)
{}

_Result::_Result(SuggestionItem item)
    : iterator(),
      suggestionItem(item.getTitle(), item.getNormalizedTitle(), item.getPath(), item.getSnippet()),
      isSuggestion(true)
{}

std::string _Result::get_url()
{
  if (isSuggestion) {
    return suggestionItem.getPath();
  }
  return iterator.getPath();
}
std::string _Result::get_title()
{
  if (isSuggestion) {
    return suggestionItem.getTitle();
  }
  return iterator.getTitle();
}
int _Result::get_score()
{
  if (isSuggestion) {
    return 0;
  }
  return iterator.getScore();
}
std::string _Result::get_snippet()
{
  if (isSuggestion) {
    return suggestionItem.getSnippet();
  }
  return iterator.getSnippet();
}
std::string _Result::get_content()
{
  if (isSuggestion) return "";
  return iterator->getItem(true).getData();
}
int _Result::get_size()
{
  if (isSuggestion) {
    return 0;
  }
  return iterator.getSize();
}
int _Result::get_wordCount()
{
  if (isSuggestion) {
    return 0;
  }
  return iterator.getWordCount();
}
std::string _Result::get_zimId()
{
  if (isSuggestion) {
    return "";
  }
  std::ostringstream s;
  s << iterator.getZimId();
  return s.str();
}


}
