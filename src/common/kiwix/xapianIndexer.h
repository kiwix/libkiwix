#ifndef KIWIX_XAPIAN_INDEXER_H
#define KIWIX_XAPIAN_INDEXER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <xapian.h>
#include <unaccent.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/fileiterator.h>
#include "xapian/myhtmlparse.h"
#include "indexer.h"

using namespace std;

namespace kiwix {
  
  class XapianIndexer : public Indexer {
    
  public:
    XapianIndexer(const string &zimFilePath, const string &xapianDirectoryPath);
    ~XapianIndexer();
    
    bool indexNextPercent(const bool &verbose = false);
    
  protected:
    void prepareIndexing();
    void stopIndexing();
    
    Xapian::WritableDatabase *writableDatabase;
    Xapian::Stem stemmer;
    Xapian::SimpleStopper stopper;
    Xapian::TermGenerator indexer;
  };

}

#endif
