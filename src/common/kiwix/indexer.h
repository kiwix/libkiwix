#ifndef KIWIX_INDEXER_H
#define KIWIX_INDEXER_H

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

using namespace std;

namespace kiwix {
  
  class Indexer {
    
  public:
    Indexer(const string &zimFilePath);
    bool indexNextPercent(const bool &verbose = false);
    
  protected:
    virtual void indexNextPercentPre() = 0;
    virtual void indexNextArticle(const string &url, 
				  const string &title, 
				  const string &unaccentedTitle,
				  const string &keywords, 
				  const string &content) = 0;
    virtual void indexNextPercentPost() = 0;
    virtual void stopIndexing() = 0;
    
    /* ZIM file handling */
    zim::File* zimFileHandler;
    zim::size_type firstArticleOffset;
    zim::size_type lastArticleOffset;
    zim::size_type currentArticleOffset;
    
    /* HTML parsing */
    MyHtmlParser htmlParser;
    unsigned int countWords(const string &text);

    /* Stopwords */
    bool readStopWordsFile(const string path);
    std::vector<std::string> stopWords;

    /* Others */
    unsigned int articleCount;
    float stepSize;
  };
}

#endif
