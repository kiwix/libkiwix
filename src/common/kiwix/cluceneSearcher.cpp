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

#include "cluceneSearcher.h"

namespace kiwix {
  
  IndexSearcher* CluceneSearcher::searcher = NULL;
  Directory* CluceneSearcher::dir = NULL;

  TCHAR buffer[MAX_BUFFER_SIZE];

  /* Constructor */
  CluceneSearcher::CluceneSearcher(const string &cluceneDirectoryPath) 
    : kiwix::Searcher() {
      if (searcher == NULL)
	this->openIndex(cluceneDirectoryPath);
  }

  /* Open Clucene readable database */
  void CluceneSearcher::openIndex(const string &directoryPath) {
    cout << "Open index folder at " << directoryPath << endl;
    dir = FSDirectory::getDirectory(directoryPath.c_str(), false);
    searcher = new IndexSearcher(dir);
  }
  
  /* Close Clucene writable database */
  void CluceneSearcher::closeIndex() {
  }
  
void CluceneSearcher::terminate()
{
  dir->close();
  searcher->close();
  delete searcher;
  _CLLDECDELETE(dir);
}
  
std::string toString(const TCHAR* s){
  int32_t len = _tcslen(s);
  char* buf = new char[len+1];
  STRCPY_WtoA(buf,s,len+1);
  string ret = buf;
  delete[] buf;
  return ret;
}
  
  /* Search strings in the database */
  void CluceneSearcher::searchInIndex(string &search, const unsigned int resultStart, 
				      const unsigned int resultEnd, const bool verbose) {
    
    // Parse query
    lucene::analysis::standard::StandardAnalyzer* analyzer = new lucene::analysis::standard::StandardAnalyzer();
    QueryParser* parser = new QueryParser(_T("content"), analyzer);
    STRCPY_AtoT(buffer, search.c_str(), MAX_BUFFER_SIZE);
    
    //lucene::util::Misc::_cpycharToWide();
    //::mbstowcs(buffer,search.c_str(),MAX_BUFFER_SIZE);
    //search.c_str();
    Query* query = parser->parse(_T("between"));
    
    cout << "Query: " << search << endl;
    wcout << "Buffer: " << buffer;
    cout << endl;
    
    const wchar_t* querystring = query->toString();
    wcout << L"Query2string: " << querystring << endl;
    
    //string q = toString(querystring);
    //STRCPY_TtoA(querystring, buffer, MAX_BUFFER_SIZE);
    //cout << "Query object: " << q << endl;
    delete[] querystring;
    
    delete parser;
    delete analyzer;

    // Search
    Hits* hits = searcher->search(query);
    cout << "Hits length:" << hits->length() << endl;
    
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
  
    searcher->close();
    
    delete searcher;
    delete hits;
    delete query;

    return;
  }
}
