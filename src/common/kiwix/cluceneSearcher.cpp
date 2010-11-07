#include "cluceneSearcher.h"

namespace kiwix {

 
  typedef std::basic_string<TCHAR> tstring;  
 
  TCHAR* StringToTCHAR(string& s)
  {
    tstring tstr;
    const char* all = s.c_str();
    int len = 1 + strlen(all);
    wchar_t* t = new wchar_t[len]; 
    if (NULL == t) throw std::bad_alloc();
    mbstowcs(t, all, len);
    return (TCHAR*)t;
  }
 
  std::string TCHARToString(const TCHAR* ptsz)
  {
    int len = wcslen((wchar_t*)ptsz);
    char* psz = new char[2*len + 1];
    wcstombs(psz, (wchar_t*)ptsz, 2*len + 1);
    std::string s = psz;
    delete [] psz;
    return s;
  }

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
    SimpleAnalyzer analyzer;
    QueryParser parser(_T("content"), &analyzer);
    Query* query = parser.parse(StringToTCHAR(search));
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
