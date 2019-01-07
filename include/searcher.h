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

#ifndef KIWIX_SEARCHER_H
#define KIWIX_SEARCHER_H

#include <stdio.h>
#include <stdlib.h>
#include <unicode/putil.h>
#include <algorithm>
#include <cctype>
#include <locale>
#include <string>
#include <vector>
#include <vector>
#include "common/pathTools.h"
#include "common/stringTools.h"
#include "kiwix_config.h"

using namespace std;

namespace kiwix
{
class Reader;
class Result
{
 public:
  virtual ~Result(){};
  virtual std::string get_url() = 0;
  virtual std::string get_title() = 0;
  virtual int get_score() = 0;
  virtual std::string get_snippet() = 0;
  virtual std::string get_content() = 0;
  virtual int get_wordCount() = 0;
  virtual int get_size() = 0;
  virtual int get_readerIndex() = 0;
};

struct SearcherInternal;
/**
 * The Searcher class is reponsible to do different kind of search using the
 * fulltext index.
 *
 *  Searcher may (if compiled with ctpp2) be used to
 *  generate a html page for the search result. This use a template that need a
 *  humanReaderName. This feature is only used by kiwix-serve and this should be
 *  move outside of Searcher (and with a better API). If you don't use the html
 *  rendering (getHtml method), you better should simply ignore the different
 *  humanReadeableName attributes (or give an empty string).
 */
class Searcher
{
 public:
  /**
   * The default constructor.
   *
   * @param humanReadableName The global zim's humanReadableName.
   *                          Used to generate pagination links.
   */
  Searcher(const string& humanReadableName = "");

  ~Searcher();

  /**
   * Add a reader (containing embedded fulltext index) to the search.
   *
   * @param reader The Reader for the zim containing the fulltext index.
   * @param humanReaderName The human readable name of the reader.
   * @return true if the reader has been added.
   *         false if the reader cannot be added (no embedded fulltext index present)
   */
  bool add_reader(Reader* reader, const std::string& humanReaderName);

  /**
   * Start a search on the zim associated to the Searcher.
   *
   * Search results should be retrived using the getNextResult method.
   *
   * @param search The search query.
   * @param resultStart the start offset of the search results (used for pagination).
   * @param resultEnd the end offset of the search results (used for pagination).
   * @param verbose print some info on stdout if true.
   */
  void search(std::string& search,
              unsigned int resultStart,
              unsigned int resultEnd,
              const bool verbose = false);

  /**
   * Start a geographique search.
   * The search return result for entry in a disc of center latitude/longitude
   * and radius distance.
   *
   * Search results should be retrived using the getNextResult method.
   *
   * @param latitude The latitude of the center point.
   * @param longitude The longitude of the center point.
   * @param distance The radius of the disc.
   * @param resultStart the start offset of the search results (used for pagination).
   * @param resultEnd the end offset of the search results (used for pagination).
   * @param verbose print some info on stdout if true.
   */
  void geo_search(float latitude, float longitude, float distance,
                  unsigned int resultStart,
                  unsigned int resultEnd,
                  const bool verbose = false);

  /**
   * Start a suggestion search.
   * The search made depend of the "version" of the embedded index.
   *  - If the index is newer enough and have a title namespace, the search is
   *    made in the titles only.
   *  - Else the search is made on the whole article content.
   * In any case, the search is made "partial" (as adding '*' at the end of the query)
   *
   * @param search The search query.
   * @param verbose print some info on stdout if true.
   */
  void suggestions(std::string& search, const bool verbose = false);

  /**
   * Get the next result of a started search.
   * This is the method to use to loop hover the search results.
   */
  Result* getNextResult();

  /**
   * Restart the previous search.
   * Next call to getNextResult will return the first result.
   */
  void restart_search();

  /**
   * Get a estimation of the result count.
   */
  unsigned int getEstimatedResultCount();

  /**
   * Set protocol prefix.
   * Only used by getHtml.
   */
  bool setProtocolPrefix(const std::string prefix);

  /**
   * Set search protocol prefix.
   * Only used by getHtml.
   */
  bool setSearchProtocolPrefix(const std::string prefix);

#ifdef ENABLE_CTPP2
  /**
   * Generate the html page with the resutls of the search.
   */
  string getHtml();
#endif

 protected:
  std::string beautifyInteger(const unsigned int number);
  void closeIndex();
  void searchInIndex(string& search,
                     const unsigned int resultStart,
                     const unsigned int resultEnd,
                     const bool verbose = false);

  std::vector<Reader*> readers;
  std::vector<std::string> humanReaderNames;
  SearcherInternal* internal;
  std::string searchPattern;
  std::string protocolPrefix;
  std::string searchProtocolPrefix;
  unsigned int resultCountPerPage;
  unsigned int estimatedResultCount;
  unsigned int resultStart;
  unsigned int resultEnd;
  std::string contentHumanReadableId;

 private:
  void reset();

};


}

#endif
