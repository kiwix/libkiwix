#ifndef KIWIX_XAPIAN_SEARCHER_H
#define KIWIX_XAPIAN_SEARCHER_H

#include <xapian.h>
#include "searcher.h"

using namespace std;

namespace kiwix {

  class XapianSearcher : public Searcher {
    
  public:
    XapianSearcher(const string &xapianDirectoryPath);

    void searchInIndex(string &search, const unsigned int resultsCount);

  protected:
    void closeIndex();
    void openIndex(const string &xapianDirectoryPath);

    Xapian::Database readableDatabase;
    Xapian::Stem stemmer;
  };

}

#endif
