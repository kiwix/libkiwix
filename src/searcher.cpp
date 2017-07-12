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
#include "kiwixlib-resources.h"
#include "reader.h"
#include "xapianSearcher.h"

#include <zim/search.h>

#ifdef ENABLE_CTPP2
#include <ctpp2/CDT.hpp>
#include <ctpp2/CTPP2FileLogger.hpp>
#include <ctpp2/CTPP2SimpleVM.hpp>
#include "ctpp2/CTPP2VMStringLoader.hpp"

using namespace CTPP;
#endif

namespace kiwix
{
class _Result : public Result
{
 public:
  _Result(Searcher* searcher, zim::Search::iterator& iterator);
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
  Searcher* searcher;
  zim::Search::iterator iterator;
};

struct SearcherInternal {
  const zim::Search* _search;
  XapianSearcher* _xapianSearcher;
  zim::Search::iterator current_iterator;

  SearcherInternal() : _search(NULL), _xapianSearcher(NULL) {}
  ~SearcherInternal()
  {
    if (_search != NULL) {
      delete _search;
    }
    if (_xapianSearcher != NULL) {
      delete _xapianSearcher;
    }
  }
};

/* Constructor */
Searcher::Searcher(const string& xapianDirectoryPath, Reader* reader)
    : reader(reader),
      internal(new SearcherInternal()),
      searchPattern(""),
      protocolPrefix("zim://"),
      searchProtocolPrefix("search://?"),
      resultCountPerPage(0),
      estimatedResultCount(0),
      resultStart(0),
      resultEnd(0)
{
  template_ct2 = RESOURCE::results_ct2;
  loadICUExternalTables();
  if (!reader || !reader->hasFulltextIndex()) {
    internal->_xapianSearcher = new XapianSearcher(xapianDirectoryPath, reader);
  }
}

/* Destructor */
Searcher::~Searcher()
{
  delete internal;
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

  /* If resultEnd & resultStart inverted */
  if (resultStart > resultEnd) {
    resultEnd += resultStart;
    resultStart = resultEnd - resultStart;
    resultEnd -= resultStart;
  }

  /* Try to find results */
  if (resultStart != resultEnd) {
    /* Avoid big researches */
    this->resultCountPerPage = resultEnd - resultStart;
    if (this->resultCountPerPage > 70) {
      resultEnd = resultStart + 70;
      this->resultCountPerPage = 70;
    }

    /* Perform the search */
    this->searchPattern = search;
    this->resultStart = resultStart;
    this->resultEnd = resultEnd;
    string unaccentedSearch = removeAccents(search);
    if (internal->_xapianSearcher) {
      internal->_xapianSearcher->searchInIndex(
          unaccentedSearch, resultStart, resultEnd, verbose);
      this->estimatedResultCount
          = internal->_xapianSearcher->results.get_matches_estimated();
    } else {
      internal->_search = this->reader->getZimFileHandler()->search(
          unaccentedSearch, resultStart, resultEnd);
      internal->current_iterator = internal->_search->begin();
      this->estimatedResultCount = internal->_search->get_matches_estimated();
    }
  }

  return;
}

void Searcher::restart_search()
{
  if (internal->_xapianSearcher) {
    internal->_xapianSearcher->restart_search();
  } else {
    internal->current_iterator = internal->_search->begin();
  }
}

Result* Searcher::getNextResult()
{
  if (internal->_xapianSearcher) {
    return internal->_xapianSearcher->getNextResult();
  } else if (internal->current_iterator != internal->_search->end()) {
    Result* result = new _Result(this, internal->current_iterator);
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

void Searcher::suggestions(std::string& search, const bool verbose)
{
  this->reset();

  if (verbose == true) {
    cout << "Performing suggestion query `" << search << "`" << endl;
  }

  this->searchPattern = search;
  this->resultStart = 0;
  this->resultEnd = 10;
  string unaccentedSearch = removeAccents(search);

  if (internal->_xapianSearcher) {
    /* [TODO] Suggestion on a external database ?
     * We do not support that. */
    this->estimatedResultCount = 0;
  } else {
    internal->_search = this->reader->getZimFileHandler()->suggestions(
          unaccentedSearch, resultStart, resultEnd);
    internal->current_iterator = internal->_search->begin();
    this->estimatedResultCount = internal->_search->get_matches_estimated();
  }
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

void Searcher::setContentHumanReadableId(const string& contentHumanReadableId)
{
  this->contentHumanReadableId = contentHumanReadableId;
}

_Result::_Result(Searcher* searcher, zim::Search::iterator& iterator)
    : searcher(searcher), iterator(iterator)
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
#ifdef ENABLE_CTPP2

string Searcher::getHtml()
{
  SimpleVM oSimpleVM;

  // Fill data
  CDT oData;
  CDT resultsCDT(CDT::ARRAY_VAL);

  this->restart_search();
  Result* p_result = NULL;
  while ((p_result = this->getNextResult())) {
    CDT result;
    result["title"] = p_result->get_title();
    result["url"] = p_result->get_url();
    result["snippet"] = p_result->get_snippet();

    if (p_result->get_size() >= 0) {
      result["size"] = kiwix::beautifyInteger(p_result->get_size());
    }

    if (p_result->get_wordCount() >= 0) {
      result["wordCount"] = kiwix::beautifyInteger(p_result->get_wordCount());
    }

    resultsCDT.PushBack(result);
    delete p_result;
  }
  this->restart_search();
  oData["results"] = resultsCDT;

  // pages
  CDT pagesCDT(CDT::ARRAY_VAL);

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
    CDT page;
    page["label"] = i + 1;
    page["start"] = i * this->resultCountPerPage;
    page["end"] = (i + 1) * this->resultCountPerPage;

    if (i * this->resultCountPerPage == this->resultStart) {
      page["selected"] = true;
    }

    pagesCDT.PushBack(page);
  }
  oData["pages"] = pagesCDT;

  oData["count"] = kiwix::beautifyInteger(this->estimatedResultCount);
  oData["searchPattern"] = kiwix::encodeDiples(this->searchPattern);
  oData["searchPatternEncoded"] = urlEncode(this->searchPattern);
  oData["resultStart"] = this->resultStart + 1;
  oData["resultEnd"] = (this->resultEnd > this->estimatedResultCount
                            ? this->estimatedResultCount
                            : this->resultEnd);
  oData["resultRange"] = this->resultCountPerPage;
  oData["resultLastPageStart"]
      = this->estimatedResultCount > this->resultCountPerPage
            ? this->estimatedResultCount - this->resultCountPerPage
            : 0;
  oData["protocolPrefix"] = this->protocolPrefix;
  oData["searchProtocolPrefix"] = this->searchProtocolPrefix;
  oData["contentId"] = this->contentHumanReadableId;

  VMStringLoader oLoader(template_ct2.c_str(), template_ct2.size());

  FileLogger oLogger(stderr);

  // DEBUG only (write output to stdout)
  // oSimpleVM.Run(oData, oLoader, stdout, oLogger);

  std::string sResult;
  oSimpleVM.Run(oData, oLoader, sResult, oLogger);

  return sResult;
}

#endif
}
