#ifndef KIWIX_XAPIAN_INDEXER_H
#define KIWIX_XAPIAN_INDEXER_H

#include <xapian.h>
#include "indexer.h"

using namespace std;

namespace kiwix {
  
  class XapianIndexer : public Indexer {
    
  public:
    XapianIndexer(const string &zimFilePath, const string &xapianDirectoryPath);
    
  protected:
    void indexNextPercentPre();
    void indexNextArticle(const string &url, 
			  const string &title, 
			  const string &unaccentedTitle,
			  const string &keywords, 
			  const string &content);
    void indexNextPercentPost();
    void stopIndexing();
    
    Xapian::WritableDatabase *writableDatabase;
    Xapian::Stem stemmer;
    Xapian::SimpleStopper stopper;
    Xapian::TermGenerator indexer;
  };

}

#endif
