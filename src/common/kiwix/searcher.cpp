#include "searcher.h"

namespace kiwix {

  /* Constructor */
  Searcher::Searcher() {
  }
  
  /* Search strings in the database */
  void Searcher::search(std::string &search, const unsigned int resultsCount, const bool verbose) {

    this->reset();
    
    if (verbose == true) {
      cout << "Performing query `" << search << "'" << endl;
    }

    searchInIndex(removeAccents(search), resultsCount, verbose);

    this->resultOffset = this->results.begin();

    return;
  }
  
  /* Reset the results */
  void Searcher::reset() {
    this->results.clear();
    this->resultOffset = this->results.begin();
    return;
  }
  
  /* Get next result */
  bool Searcher::getNextResult(string &url, string &title, unsigned int &score) {
    bool retVal = false;
    
    if (this->resultOffset != this->results.end()) {
      
      /* url */
      url = this->resultOffset->url;
      
      /* title */
      title = this->resultOffset->title;
      
      /* score */
      score =  this->resultOffset->score;
      
      /* increment the cursor for the next call */
      this->resultOffset++;
      
      retVal = true;
    }

    return retVal;
  }
  
}
