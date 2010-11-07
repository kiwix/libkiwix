#include "xapianSearcher.h"

namespace kiwix {

  /* Constructor */
  XapianSearcher::XapianSearcher(const string &xapianDirectoryPath) 
    : Searcher(),
      stemmer(Xapian::Stem("english")) {
    this->openIndex(xapianDirectoryPath);
  }

  /* Open Xapian readable database */
  void XapianSearcher::openIndex(const string &directoryPath) {
    this->readableDatabase = Xapian::Database(directoryPath);
  }
  
  /* Close Xapian writable database */
  void XapianSearcher::closeIndex() {
    return;
  }
  
  /* Search strings in the database */
  void XapianSearcher::searchInIndex(string &search, const unsigned int resultsCount, const bool verbose) {
    /* Create the query */
    Xapian::QueryParser queryParser;
    Xapian::Query query = queryParser.parse_query(search);    

    /* Create the enquire object */
    Xapian::Enquire enquire(this->readableDatabase);
    enquire.set_query(query);

    /* Get the results */
    Xapian::MSet matches = enquire.get_mset(0, resultsCount);
    
    Xapian::MSetIterator i;
    for (i = matches.begin(); i != matches.end(); ++i) {
      Xapian::Document doc = i.get_document();
      
      Result result;
      result.url = doc.get_data();
      result.title = doc.get_value(0);
      result.score = i.get_percent();
      
      this->results.push_back(result);

      if (verbose) {
	std::cout << "Document ID " << *i << "   \t";
	std::cout << i.get_percent() << "% ";
	std::cout << "\t[" << doc.get_data() << "] - " << doc.get_value(0) << std::endl;
      }

    }

    return;
  }
}
