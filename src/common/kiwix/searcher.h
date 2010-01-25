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
#include <xapian.h>
#include <unaccent.h>
#include <splitString.h>

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
    Searcher(const string &xapianDirectoryPath);
    ~Searcher();

    Xapian::Database readableDatabase;
    Xapian::Stem stemmer;
    std::vector<Result> results;
    std::vector<Result>::iterator resultOffset;
    
    void search(string search, const unsigned int resultsCount);
    bool getNextResult(string &url, string &title, unsigned int &score);
    void closeDatabase();
    void reset();

  protected:
    void openDatabase(const string &xapianDirectoryPath);
  };

}

#endif
