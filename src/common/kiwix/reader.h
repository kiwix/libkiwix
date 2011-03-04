#ifndef KIWIX_READER_H
#define KIWIX_READER_H

#include <zim/zim.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/fileiterator.h>
#include <string>
#include <sstream>
#include "time.h"

using namespace std;

namespace kiwix {

  class Reader {
    
  public:
    Reader(const string &zimFilePath);
    ~Reader();

    void reset();
    unsigned int getArticleCount();
    string getId();
    string getRandomPageUrl();
    string getFirstPageUrl();
    string getMainPageUrl();
    bool getMetatag(const string &url, string &content);
    bool getContentByUrl(const string &url, string &content, unsigned int &contentLength, string &contentType);
    bool searchSuggestions(const string &prefix, unsigned int suggestionsCount);
    bool getNextSuggestion(string &title);
    bool canCheckIntegrity();
    bool isCorrupted();

  protected:
    zim::File* zimFileHandler;
    zim::size_type firstArticleOffset;
    zim::size_type lastArticleOffset;
    zim::size_type currentArticleOffset;
    zim::size_type articleCount;
    
    std::vector<std::string> suggestions;
    std::vector<std::string>::iterator suggestionsOffset;
  };

}

#endif
