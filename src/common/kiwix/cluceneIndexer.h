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

#ifndef KIWIX_CLUCENE_INDEXER_H
#define KIWIX_CLUCENE_INDEXER_H

#include <CLucene.h>
#include "indexer.h"

#define MAX_BUFFER_SIZE 4200000

using namespace std;

using namespace lucene::analysis;
using namespace lucene::index;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::search;
using namespace lucene::store;

namespace kiwix {
  
  class CluceneIndexer : public Indexer {
    
  public:
    CluceneIndexer(const string &zimFilePath, const string &cluceneDirectoryPath);
    
  protected:
    void indexNextPercentPre();
    void indexNextArticle(const string &url, 
			  const string &title, 
			  const string &unaccentedTitle,
			  const string &keywords, 
			  const string &content);
    void indexNextPercentPost();
    void stopIndexing();

    FSDirectory* dir;
    IndexWriter* writer;
    lucene::analysis::standard::StandardAnalyzer analyzer;
  };

}

#endif
