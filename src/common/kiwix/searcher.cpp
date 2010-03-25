#include "searcher.h"

namespace kiwix {

  /* Constructor */
  Searcher::Searcher(const string &xapianDirectoryPath) 
    : stemmer(Xapian::Stem("english")) {
    this->openDatabase(xapianDirectoryPath);
  }
  
  /* Destructor */
  Searcher::~Searcher() {
  }

  /* Open Xapian readable database */
  void Searcher::openDatabase(const string &directoryPath) {
    this->readableDatabase = Xapian::Database(directoryPath);
  }
  
  /* Close Xapian writable database */
  void Searcher::closeDatabase() {
    return;
  }
  
  /* Search strings in the database */
  void Searcher::search(string search, const unsigned int resultsCount, bool verbose) {
    /* Reset the results */
    this->results.clear();
    this->resultOffset = this->results.begin();
    
    /* Create the enquire object */
    Xapian::Enquire enquire(this->readableDatabase);

    /* lowercase the search pattern */
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    
    /* Create the query term vector */
    /* I have the doublequote " because bug ID: 2939690 */
    std::vector<std::string> queryTerms = split(removeAccents(search.c_str()), " #@%$0/\\_-*()[]{},;:\"´`'");
    
    /* Create query object */
    Xapian::Query query(Xapian::Query::OP_OR, queryTerms.begin(), queryTerms.end());
    
    /* Set the query in the enquire object */
    enquire.set_query(query);
    
    if (verbose == true) {
      cout << "Performing query `" <<
	query.get_description() << "'" << endl;
    }

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
      
      if (verbose == true) {
	cout << "Document ID " << *i << "   \t";
	cout << i.get_percent() << "% ";
	cout << "\t[" << doc.get_data() << "] - " << doc.get_value(0) << endl;
      }
    }
    
    /* Set the cursor to the begining */
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
