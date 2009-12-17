#ifndef KIWIX_INDEXER_H
#define KIWIX_INDEXER_H

#include <xapian.h>
#include <unaccent.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/fileiterator.h>
#include "xapian/myhtmlparse.h"

using namespace std;

namespace kiwix {

  class Indexer {
    
  public:
    Indexer(const string &zimFilePath, const string &xapianDirectoryPath);
    ~Indexer();    
    
    void startIndexing();
    bool indexNextPercent();
    void stopIndexing();

  protected:
    zim::File* zimFileHandler;
    zim::size_type firstArticleOffset;
    zim::size_type lastArticleOffset;
    zim::size_type currentArticleOffset;
    zim::Article currentArticle;
    
    unsigned int articleCount;
    float stepSize;
    
    Xapian::WritableDatabase writableDatabase;
    Xapian::Stem stemmer;
    Xapian::TermGenerator indexer;
    MyHtmlParser htmlParser;
  };

}

#endif
