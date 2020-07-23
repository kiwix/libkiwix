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
Searcher::Searcher()
    : internal(new SearcherInternal()),
      searchPattern(""),
      estimatedResultCount(0),
      resultStart(0),
      resultEnd(0)
{
  loadICUExternalTables();
}

/* Destructor */
Searcher::~Searcher()
{
  delete internal;
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
                      unsigned int resultEnd,
                      const bool verbose)
{
  this->reset();

  if (verbose == true) {
    cout << "Performing query `" << search << "'" << endl;
  }

  this->searchPattern = search;
  this->resultStart = resultStart;
  this->resultEnd = resultEnd;
  /* Try to find results */
  if (resultStart != resultEnd) {
    /* Perform the search */
    string unaccentedSearch = removeAccents(search);
    std::vector<zim::Archive> archives;
    for (auto current = this->readers.begin(); current != this->readers.end();
         current++) {
      if ( (*current)->hasFulltextIndex() ) {
          archives.push_back(*(*current)->getZimArchive());
      }
    }
    zim::Search* search = new zim::Search(archives);
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

  /* Perform the search */
  std::ostringstream oss;
  oss << "Articles located less than " << distance << " meters of " << latitude << ";" << longitude;
  this->searchPattern = oss.str();
  this->resultStart = resultStart;
  this->resultEnd = resultEnd;

  /* Try to find results */
  if (resultStart == resultEnd) {
    return;
  }

  std::vector<zim::Archive> archives;
  for (auto current = this->readers.begin(); current != this->readers.end();
       current++) {
    archives.push_back(*(*current)->getZimArchive());
  }
  zim::Search* search = new zim::Search(archives);
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

  std::vector<zim::Archive> archives;
  for (auto current = this->readers.begin(); current != this->readers.end();
       current++) {
    archives.push_back(*(*current)->getZimArchive());
  }
  zim::Search* search = new zim::Search(archives);
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
  return iterator->getItem(true).getData();
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


}
