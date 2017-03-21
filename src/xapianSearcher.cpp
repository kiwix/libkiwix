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

#include "xapianSearcher.h"
#include <zim/zim.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/error.h>
#include <sys/types.h>
#include <unistd.h>

namespace kiwix {

  /* Constructor */
  XapianSearcher::XapianSearcher(const string &xapianDirectoryPath) 
    : Searcher(),
      stemmer(Xapian::Stem("english")) {
    this->openIndex(xapianDirectoryPath);
  }

  /* Open Xapian readable database */
  void XapianSearcher::openIndex(const string &directoryPath) {
    try
    {
      zim::File zimFile = zim::File(directoryPath);
      zim::Article xapianArticle = zimFile.getArticle('Z', "/fulltextIndex/xapian");
      if (!xapianArticle.good())
	throw NoXapianIndexInZim();
      zim::offset_type dbOffset = xapianArticle.getOffset();
      int databasefd = open(directoryPath.c_str(), O_RDONLY);
      lseek(databasefd, dbOffset, SEEK_SET);
      this->readableDatabase = Xapian::Database(databasefd);
    } catch (...) {
      this->readableDatabase = Xapian::Database(directoryPath);
    }
  }
  
  /* Close Xapian writable database */
  void XapianSearcher::closeIndex() {
    return;
  }
  
  /* Search strings in the database */
  void XapianSearcher::searchInIndex(string &search, const unsigned int resultStart, 
				     const unsigned int resultEnd, const bool verbose) {
    /* Create the query */
    Xapian::QueryParser queryParser;
    Xapian::Query query = queryParser.parse_query(search);    

    /* Create the enquire object */
    Xapian::Enquire enquire(this->readableDatabase);
    enquire.set_query(query);

    /* Get the results */
    this->results = enquire.get_mset(resultStart, resultEnd - resultStart);
    this->current_result = this->results.begin();

    /* Update the global resultCount value*/
    this->estimatedResultCount = this->results.get_matches_estimated();
  }

  /* Get next result */
  Result* XapianSearcher::getNextResult() {
    if (this->current_result != this->results.end()) {
      XapianResult* result = new XapianResult(this->current_result);
      this->current_result++;
      return result;
    }
    return NULL;
  }

  void XapianSearcher::restart_search() {
    this->current_result = this->results.begin();
  }

  XapianResult::XapianResult(Xapian::MSetIterator& iterator):
    iterator(iterator),
    document(iterator.get_document())
  {
  }

  std::string XapianResult::get_url() {
    return document.get_data();
  }

  std::string XapianResult::get_title() {
      return document.get_value(0);
  }

  int XapianResult::get_score() {
    return iterator.get_percent();
  }

  std::string XapianResult::get_snippet() {
      return document.get_value(1);
  }

  int XapianResult::get_size() {
      return document.get_value(2).empty() == true ? -1 : atoi(document.get_value(2).c_str());
  }

  int XapianResult::get_wordCount() {
        return document.get_value(3).empty() == true ? -1 : atoi(document.get_value(3).c_str());
  }

} // Kiwix namespace
