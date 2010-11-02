#ifndef KIWIX_CLUCENE_INDEXER_H
#define KIWIX_CLUCENE_INDEXER_H

#include <CLucene.h>
#include "indexer.h"

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
    SimpleAnalyzer analyzer;
  };

}

#endif
