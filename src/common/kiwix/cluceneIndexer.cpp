#include "cluceneIndexer.h"

namespace kiwix {

  CluceneIndexer::CluceneIndexer(const string &zimFilePath, const string &cluceneDirectoryPath) :
    Indexer(zimFilePath) {
  }
  
  void CluceneIndexer::indexNextPercentPre() {
  }
  
  void CluceneIndexer::indexNextArticle(string &url, string &title, string &unaccentedTitle,
				       string &keywords, string &content) {
    
  }

  void CluceneIndexer::indexNextPercentPost() {
  }
  
  void CluceneIndexer::stopIndexing() {
  }
}
