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

#ifndef KIWIX_CLUCENE_SEARCHER_H
#define KIWIX_CLUCENE_SEARCHER_H

#include <CLucene.h>
#include <CLucene/queryParser/MultiFieldQueryParser.h>
#include "searcher.h"

#define MAX_BUFFER_SIZE 4200000

using namespace std;
using namespace lucene::analysis;
using namespace lucene::index;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::search;
using namespace lucene::store;

namespace kiwix {

  class CluceneSearcher : public Searcher {
    
  public:
    CluceneSearcher(const string &cluceneDirectoryPath);

    void searchInIndex(string &search, const unsigned int resultsCount, const bool verbose=false);

  protected:
    void closeIndex();
    void openIndex(const string &cluceneDirectoryPath);

    IndexReader* reader;
    lucene::analysis::standard::StandardAnalyzer analyzer;

  };

}

#endif
