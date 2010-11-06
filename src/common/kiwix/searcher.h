#ifndef KIWIX_SEARCHER_H
#define KIWIX_SEARCHER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <locale>
#include <cctype>
#include <vector>
#include <unaccent.h>

using namespace std;

struct Result
{
  string url;
  string title;
  int score;
}; 

namespace kiwix {

  class Searcher {
    
  public:
    Searcher();

    void search(std::string &search, const unsigned int resultsCount, bool verbose=false);
    bool getNextResult(string &url, string &title, unsigned int &score);
    void reset();

  protected:
    virtual void closeIndex() = 0;
    virtual void searchInIndex(string &search, const unsigned int resultsCount) = 0;

    std::vector<Result> results;
    std::vector<Result>::iterator resultOffset;
  };

}

#endif
