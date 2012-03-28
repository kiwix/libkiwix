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

#ifndef KIWIX_INDEXER_H
#define KIWIX_INDEXER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

#include <xapian.h>
#include <pthread.h>
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
    bool setZimFilePath(const string &zimFilePath);
    bool start();
    bool stop();
    bool isRunning();
    unsigned int getProgression();

  private:
    pthread_t articleExtracter, articleParser, indexWriter;
    static void *extractArticles(void *ptr);
    static void *parseArticles(void *ptr);
    static void *writeIndex(void *ptr);
    
    unsigned int runningStatus;
    void incrementRunningStatus();
    void decrementRunningStatus();
    unsigned int getRunningStatus();

  protected:
    virtual void indexNextPercentPre() = 0;
    virtual void indexNextArticle(const string &url, 
				  const string &title, 
				  const string &unaccentedTitle,
				  const string &keywords, 
				  const string &content,
				  const string &snippet,
				  const string &size,
				  const string &wordCount) = 0;
    virtual void indexNextPercentPost() = 0;
    virtual void stopIndexing() = 0;

    /* Article offset */
    void setCurrentArticleOffset(unsigned int offset);
    unsigned int getCurrentArticleOffset();

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

    /* Boost factor */
    unsigned int keywordsBoostFactor;
    inline unsigned int getTitleBoostFactor(const unsigned int contentLength) {
      return contentLength / 500 + 1;
    }
  };
}

#endif
