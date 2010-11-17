#include "cluceneSearcher.h"

namespace kiwix {

  TCHAR buffer[MAX_BUFFER_SIZE];

  /* Constructor */
  CluceneSearcher::CluceneSearcher(const string &cluceneDirectoryPath) 
    : kiwix::Searcher() {
    this->openIndex(cluceneDirectoryPath);
  }

  /* Open Clucene readable database */
  void CluceneSearcher::openIndex(const string &directoryPath) {
    this->reader = IndexReader::open(directoryPath.c_str());
  }
  
  /* Close Clucene writable database */
  void CluceneSearcher::closeIndex() {
    return;
  }
  
  /* Search strings in the database */
  void CluceneSearcher::searchInIndex(string &search, const unsigned int resultsCount, const bool verbose) {
    IndexSearcher searcher(reader);
    QueryParser parser(_T("content"), &analyzer);
    STRCPY_AtoT(buffer, search.c_str(), MAX_BUFFER_SIZE);
    Query* query = parser.parse(buffer);
    Hits* hits = searcher.search(query);
    cout << "--------------------------------" << hits->length() << endl;
    
    /*
    for (size_t i=0; i < hits->length() && i<10; i++) {
      Document* d = &hits->doc(i);
      _tprintf(_T("#%d. %s (score: %f)\n"),
	       i, d->get(_T("contents")),
	       hits->score(i));
    }
    */
    /*
      Result result;
      result.url = doc.get_data();
      result.title = doc.get_value(0);
      result.score = i.get_percent();
      
      this->results.push_back(result);
    */

    return;
  }
}
