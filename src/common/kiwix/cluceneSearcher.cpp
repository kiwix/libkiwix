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
  void CluceneSearcher::searchInIndex(string &search, const unsigned int resultStart, 
				      const unsigned int resultEnd, const bool verbose) {
    IndexSearcher searcher(reader);
    QueryParser parser(_T("content"), &analyzer);
    //STRCPY_AtoT(buffer, search.c_str(), MAX_BUFFER_SIZE);
    ::mbstowcs(buffer,search.c_str(),MAX_BUFFER_SIZE);
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
