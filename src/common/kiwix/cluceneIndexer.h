#ifndef KIWIX_CLUCENE_INDEXER_H
#define KIWIX_CLUCENE_INDEXER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <unaccent.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/fileiterator.h>
#include "xapian/myhtmlparse.h"
#include "indexer.h"

using namespace std;

namespace kiwix {
  
  class CluceneIndexer : public Indexer {
    
  public:
    CluceneIndexer(const string &zimFilePath, const string &cluceneDirectoryPath);
    
  protected:
    void indexNextPercentPre();
    void indexNextArticle(string &url, string &title, string &unaccentedTitle,
			  string &keywords, string &content);
    void indexNextPercentPost();
    void stopIndexing();
  };

}

#endif
