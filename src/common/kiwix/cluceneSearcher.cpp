#include "cluceneSearcher.h"

namespace kiwix {

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
  void CluceneSearcher::searchInIndex(string &search, const unsigned int resultsCount) {
    IndexSearcher searcher(reader);
    SimpleAnalyzer analyzer;
    QueryParser parser(_T("content"), &analyzer);
    Query* query = parser.parse((const wchar_t*)search.c_str());
    Hits* hits = searcher.search(query);

    for (size_t i=0; i < hits->length() && i<10; i++) {
      Document* d = &hits->doc(i);
      _tprintf(_T("#%d. %s (score: %f)\n"),
	       i, d->get(_T("contents")),
	       hits->score(i));
    }

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
