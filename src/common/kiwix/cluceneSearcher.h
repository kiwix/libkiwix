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
